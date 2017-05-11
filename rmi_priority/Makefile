make: clean build run

run: start stop

start: registry.PID server.PID

reboot: stop clean build start

registry.PID:
	@echo STARTING RMI REGISTRY
	@cd build && { rmiregistry & echo $$! > ../$@; }
	@sleep 2

server.PID: registry.PID
	@echo STARTING PRIORITY SERVER
	@java -classpath build -Djava.rmi.server.codebase=file:build/ Server & echo $$! > $@
	@sleep 2

client: server.PID
	@java -classpath build Client $(fk) $(ct) $(l)
	@sleep 2

stop:
	@echo KILLING RMI REGISTRY
	@kill `cat registry.PID` && rm registry.PID
	
	@echo KILLING RMI SERVER
	@kill `cat server.PID` && rm server.PID

build: Client.java Resource.java Server.java
	@echo BUILDING PROJECT
	@mkdir build
	@javac -d build Client.java Resource.java Server.java

clean:
	@echo CLEANING PROJECT
	@rm -R build
	@rm *.txt

test-parallel-write:
	@make client fk=2 ct=content1 l=3 &
	@make client fk=1 ct=content2 l=3 &

test-read-priority:
	@make client fk=1 ct=pepe1 l=11 &
	@sleep 3
	@echo
	@make client fk=1 l=18 &
	@sleep 6
	@echo
	@make client fk=1 ct=pepe2 l=6 &
	@sleep 6
	@echo
	@make client fk=1 l=18 &

.PHONY: run start stop build clean client