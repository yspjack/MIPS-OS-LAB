// implement fork from user space

#include "lib.h"
#include <mmu.h>
#include <env.h>
// #define DEBUG(...) writef(__VA_ARGS__)
#define DEBUG(...)
/* ----------------- help functions ---------------- */

/* Overview:
 * 	Copy `len` bytes from `src` to `dst`.
 *
 * Pre-Condition:
 * 	`src` and `dst` can't be NULL. Also, the `src` area
 * 	 shouldn't overlap the `dest`, otherwise the behavior of this
 * 	 function is undefined.
 */
void user_bcopy(const void *src, void *dst, size_t len)
{
    void *max;

    //	writef("~~~~~~~~~~~~~~~~ src:%x dst:%x len:%x\n",(int)src,(int)dst,len);
    max = dst + len;

    // copy machine words while possible
    if (((int)src % 4 == 0) && ((int)dst % 4 == 0))
    {
        while (dst + 3 < max)
        {
            *(int *)dst = *(int *)src;
            dst += 4;
            src += 4;
        }
    }

    // finish remaining 0-3 bytes
    while (dst < max)
    {
        *(char *)dst = *(char *)src;
        dst += 1;
        src += 1;
    }

    //for(;;);
}

/* Overview:
 * 	Sets the first n bytes of the block of memory
 * pointed by `v` to zero.
 *
 * Pre-Condition:
 * 	`v` must be valid.
 *
 * Post-Condition:
 * 	the content of the space(from `v` to `v`+ n)
 * will be set to zero.
 */
void user_bzero(void *v, u_int n)
{
    char *p;
    int m;

    p = v;
    m = n;

    while (--m >= 0)
    {
        *p++ = 0;
    }
}
/*--------------------------------------------------------------*/

/* Overview:
 * 	Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 *
 * Pre-Condition:
 * 	`va` is the address which leads to a TLBS exception.
 *
 * Post-Condition:
 *  Launch a user_panic if `va` is not a copy-on-write page.
 * Otherwise, this handler should map a private writable copy of
 * the faulting page at correct address.
 */
static void
pgfault(u_int va)
{
    u_int tmp;
    u_int PFTEMP = (u_int)(UTEXT-2*BY2PG);
    // writef("fork.c:pgfault():\t va:%x\n", va);
    if (!((*vpt)[VPN(va)] & PTE_COW))
    {
        user_panic("not a copy-on-write page: 0x%x\n", va);
    }
    //map the new page at a temporary place
    syscall_mem_alloc(0, PFTEMP, PTE_V | PTE_R);
    //copy the content
    tmp = ROUNDDOWN(va, BY2PG);
    user_bcopy((void *)tmp, (void *)PFTEMP, BY2PG);
    //map the page on the appropriate place
    syscall_mem_map(0, PFTEMP, 0, va, PTE_V | PTE_R);
    //unmap the temporary place
    syscall_mem_unmap(0, PFTEMP);
}

/* Overview:
 * 	Map our virtual page `pn` (address pn*BY2PG) into the target `envid`
 * at the same virtual address.
 *
 * Post-Condition:
 *  if the page is writable or copy-on-write, the new mapping must be
 * created copy on write and then our mapping must be marked
 * copy on write as well. In another word, both of the new mapping and
 * our mapping should be copy-on-write if the page is writable or
 * copy-on-write.
 *
 * Hint:
 * 	PTE_LIBRARY indicates that the page is shared between processes.
 * A page with PTE_LIBRARY may have PTE_R at the same time. You
 * should process it correctly.
 */
static void
duppage(u_int envid, u_int pn)
{
    u_int addr;
    u_int perm;

    // user_panic("duppage not implemented");
    addr = pn * BY2PG;
    perm = ((*vpt)[pn]) & 0xfff;
    if ((perm & PTE_V) && ((perm & PTE_R) || (perm & PTE_COW)))
    {
        DEBUG("duppage: new mapping 0x%x\n", pn << 12);
        if (perm & PTE_LIBRARY)
        {
            perm = PTE_V | PTE_R | PTE_LIBRARY;
        }
        else
        {
            perm = PTE_V | PTE_COW;
        }
        if (syscall_mem_map(0, addr, envid, addr, perm) < 0)
        {
            user_panic("duppage: map failed\n");
        }
        if (syscall_mem_map(0, addr, 0, addr, perm) < 0)
        {
            user_panic("duppage: map failed\n");
        }
    }
    else
    {
        if (syscall_mem_map(0, addr, envid, addr, perm) < 0)
        {
            user_panic("duppage: map failed\n");
        }
    }
}

/* Overview:
 * 	User-level fork. Create a child and then copy our address space
 * and page fault handler setup to the child.
 *
 * Hint: use vpd, vpt, and duppage.
 * Hint: remember to fix "env" in the child process!
 * Note: `set_pgfault_handler`(user/pgfault.c) is different from
 *       `syscall_set_pgfault_handler`.
 */
extern void __asm_pgfault_handler(void);
int fork(void)
{
    // Your code here.
    u_int newenvid;
    extern struct Env *envs;
    extern struct Env *env;
    u_int i;

    //The parent installs pgfault using set_pgfault_handler
    set_pgfault_handler(pgfault);
    DEBUG("fork: set_pgfault_handler\n");
    //alloc a new alloc
    newenvid = syscall_env_alloc();
    DEBUG("fork: syscall_env_alloc\n");
    if (newenvid < 0)
    {
        DEBUG("fork: envid error\n");
        return newenvid;
    }
    if (newenvid == 0)
    {
        //child
        DEBUG("fork::child\n");
        env = &envs[ENVX(syscall_getenvid())];
        return 0;
    }
    DEBUG("fork::parent\n");
    for (i = 0; i < (UTOP / BY2PG) - 2; ++i)
    {
        if (((*vpd)[i >> 10]) && ((*vpt)[i]))
        {
            duppage(newenvid, i);
        }
    }
    DEBUG("fork::parent: duppage\n");

    syscall_mem_alloc(newenvid, UXSTACKTOP - BY2PG, PTE_V | PTE_R);
    DEBUG("fork::parent: syscall_mem_alloc UXSTACKTOP\n");

    syscall_set_pgfault_handler(newenvid, __asm_pgfault_handler, UXSTACKTOP);
    DEBUG("fork::parent: syscall_set_pgfault_handler\n");

    syscall_set_env_status(newenvid, ENV_RUNNABLE);
    DEBUG("fork::parent: syscall_set_env_status\n");
    DEBUG("fork::parent: newenvid: %d\n", newenvid);
    return newenvid;
}

// Challenge!
int sfork(void)
{
    user_panic("sfork not implemented");
    return -E_INVAL;
}
