#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <casey/types.h>
#include <sys/msgno.h>

static 
int splitargv ( char *src,char argv[][30],int max_nr ) 
{
    int i = 0,j = 0 ,len = strlen (src),cmd_cnt;

    while ( src[i] == ' ' ) i ++;

    for ( cmd_cnt = 0; src[i] && cmd_cnt < max_nr; i ++ ){
        if (src[i] != ' ' ){
            argv[cmd_cnt][j ++] = src[i];
            continue;
        }
        else{
            argv[cmd_cnt][j] = '\0';
            cmd_cnt ++;
            j = 0;
        }
    }

    argv[cmd_cnt][j] = '\0';
    cmd_cnt ++;

    return (cmd_cnt);
}

void echo ( int argc,char argv[][30] ) 
{
    int i = 0;

    for ( i = 1 ; i < argc ; i ++ ) 
    {
        printf ( "%s ",argv[i] );
    }
    printf ("\n" );
}


/* main line */
int main ( void )
{
    startup (); /* fs */
    startup (); /* tty */
    startup (); /* mm */


    delay_ms (1000);

    /* start shell */
    char    a[100] = {0};
    int     cmd_cnt = 0,i = 0;
    char    argv[30][30] = {0};

    printf ( "mailbox = %d \n",sizeof(MAILBOX) );
    MAIL m;
    while ( true ) {
        delay_ms (500);
        getm (&m,MSG_ANY|MSG_PST);
        printf ( "msg(%d) = %d ",i++,m.m_msg );
    }

    pause ();

    while (1)
    {
        printf ( "casey # " );
        _gets (a,100);

        cmd_cnt = splitargv (a,argv,30);

        if ( !strcmp ("echo",argv[0]) ){
            echo (cmd_cnt,argv);
        }else if ( !strcmp ("hello",argv[0]) ) {
            char (*p)[30] = argv;
            exec (cmd_cnt,p,900);
        }
        else 
            printf ( "command not recognised ! \n" );
    }
}

