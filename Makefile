CC = gcc
CFLAGS = -std=c99 -pedantic -O2 -Wall -ggdb
NAME = smtpparser.so

.PHONY: clean

all: $(NAME)

$(NAME): smtp_parser.o parser.o
	$(CC) $(CFLAGS) -shared -o $(NAME) $^

smtp_parser.o: smtp_parser.c smtp_parser.h
	$(CC) $(CFLAGS) -fpic -c $<

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -fpic -c $<

clean:
	rm -f $(NAME) *.o

