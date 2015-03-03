/**
 * @file bug626.cpp  regression case for bug 626 ("Crash when user define with old password style (before 4.1 protocol)"), also checks error message in the log for bug428 ("Pre MySQL 4.1 encrypted passwords cause authorization failure")
 *
 * - CREATE USER 'old'@'%' IDENTIFIED BY 'old';
 * - SET PASSWORD FOR 'old'@'%' = OLD_PASSWORD('old');
 * - try to connect using user 'old'
 * - check log for "MaxScale does not support these old passwords" warning
 * - DROP USER 'old'@'%'
 * - check MaxScale is alive
 */

#include <my_config.h>
#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int global_result = 0;

    Test->read_env();
    Test->print_env();

    Test->connect_maxscale();

    printf("Creating user with old style password\n");
    global_result += execute_query(Test->conn_rwsplit, (char *) "CREATE USER 'old'@'%' IDENTIFIED BY 'old';");
    global_result += execute_query(Test->conn_rwsplit, (char *) "SET PASSWORD FOR 'old'@'%' = OLD_PASSWORD('old');");
    sleep(10);

    printf("Trying to connect using user with old style password\n");
    MYSQL * conn = open_conn(Test->rwsplit_port, Test->maxscale_IP, (char *) "old", (char *)  "old");

    if ( conn == NULL) {
        printf("Connections is not open as expected\n");
    } else {
        printf("Connections is open for the user with old style password. FAILED!\n");
        global_result++;
        mysql_close(conn);
    }

    global_result += execute_query(Test->conn_rwsplit, (char *) "DROP USER 'old'@'%'");
    Test->close_maxscale_connections();

    global_result += check_log_err((char *) "MaxScale does not support these old passwords", TRUE);

    global_result += check_maxscale_alive();

    Test->copy_all_logs(); return(global_result);
}

