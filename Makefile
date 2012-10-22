
all: compile test

compile:
	./rebar compile

test: compile 
	./rebar eunit

clean: test-clean
	./rebar clean
