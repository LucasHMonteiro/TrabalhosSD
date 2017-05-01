make: build run

run: start stop

start: registry.PID server.PID client

registry.PID:
	@echo
	@echo STARTING RMI REGISTRY
	cd build && { rmiregistry & echo $$! > ../$@; }
	@sleep 2
	@echo

server.PID: registry.PID
	@echo STARTING HELLO SERVER
	java -classpath build -Djava.rmi.server.codebase=file:build/ example.hello.Server & echo $$! > $@
	@sleep 2
	@echo

client: server.PID
	@echo STARTING HELLO CLIENT
	java -classpath build example.hello.Client
	@sleep 2
	@echo

stop:
	@echo KILLING RMI REGISTRY
	kill `cat registry.PID` && rm registry.PID
	@echo
	
	@echo KILLING RMI SERVER
	kill `cat server.PID` && rm server.PID
	@echo

build: Client.java Hello.java Server.java
	mkdir build
	javac -d build Client.java Hello.java Server.java

clean:
	rm -R build

.PHONY: run start stop build clean client