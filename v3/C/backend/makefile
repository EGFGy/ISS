DEPS=SQL_functions_v1.o CGI_functions.o
CC=gcc
CFLAGS=-Wextra -Wall -DGNU_SOURCE -Wextra -Wall -g -std=gnu99 -I/usr/include/mysql
LIBS=-lmysqlclient -lm -lcrypt
OBJS=.objs/CGI_functions.o .objs/SQL_functions_v1.o

%.o: %.c
	$(CC) -c -o .objs/$@ $< $(CFLAGS)

timetable: timetable.o $(DEPS)
	$(CC)  -o bin/Debug/timetable bin/Debug/CGI_functions.o bin/Debug/SQL_functions_v1.o bin/Debug/timetable.o $(LIBS)

logout: logout.o $(DEPS)
	$(CC) $(CFLAGS) .objs/logout.o $(OBJS) -o bin/Debug/logout.cgi  $(LIBS)

login: main_strtok_base3.o $(DEPS)
	$(CC) $(CFLAGS) .objs/main_strtok_base3.o $(OBJS) -o bin/Debug/login.cgi $(LIBS)

register: register.o  $(DEPS)
	$(CC) $(CFLAGS) .objs/register.o $(OBJS) -o bin/Debug/register.cgi $(LIBS)

all_messages: all_messages.o $(DEPS)
	$(CC) $(CFLAGS) .objs/all_messages.o $(OBJS) -o bin/Debug/all_messages.cgi $(LIBS)

check_user_exist: check_user_exist.o $(DEPS)
	$(CC) $(CFLAGS) .objs/check_user_exist.o $(OBJS) -o bin/Debug/check_user_exist.cgi $(LIBS)

main_strtok_base3: main_strtok_base3.o $(DEPS)
	$(CC) $(CFLAGS) .objs/main_strtok_base3.o $(OBJS) -o bin/Debug/main_strtok_base3.cgi $(LIBS)

post_message: post_message.o $(DEPS)
	$(CC) $(CFLAGS) .objs/post_message.o $(OBJS) -o bin/Debug/post_message.cgi $(LIBS)

settings: settings.o $(DEPS)
	$(CC) $(CFLAGS) .objs/settings.o $(OBJS) -o bin/Debug/settings.cgi $(LIBS)

spec_messages: spec_messages.o $(DEPS)
	$(CC) $(CFLAGS) .objs/spec_messages.o $(OBJS) -o bin/Debug/spec_messages.cgi $(LIBS)

timetable: timetable.o $(DEPS)
	$(CC) $(CFLAGS) .objs/timetable.o $(OBJS) -o bin/Debug/timetable.cgi $(LIBS)
