#include<stdio.h>

int fibo(int n){
	int i;
 	int a=1,b=1,c=2;
	if(n<2){
		for(i=0;i<n;++i){
			printf("1 ");
		}
	}else {
		for(i=0;i<2;++i){
			printf("1 ");
		}
		for(i=2;i<n;++i){
			printf("%d ",c);
			a=b;
			b=c;
			c=a+b;
		}
	}
}

int main(){
	int var;
	scanf("%d", &var);
	fibo(var);
        return 0;
}
