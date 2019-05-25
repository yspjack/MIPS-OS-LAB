#include "lib.h"


static char buf[512];

static void user_out2string(void *arg, char *s, int l)
{
	int i;
	static char *b = buf;
	// special termination call
	if (l == 0)
		return;
	if ((l == 1) && (s[0] == '\0'))
	{
		*b = '\0';
		b = buf;
		return;
	}
	for (i = 0; i < l; i++)
	{
		*b++ = s[i];
	}
	*b++ = '\0';
}


int fwritef(int fd, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	user_lp_Print(user_out2string, (void*)0, fmt, ap);
	va_end(ap);
	// writef("\n[fwritef]write(%d,buf,%d)\n", fd, strlen(buf));
	// writef("ch:%d\n", buf[0]);
	return write(fd, buf, strlen(buf));
}
