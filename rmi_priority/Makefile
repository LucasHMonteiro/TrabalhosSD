make: clean build run

run: start stop

start: registry.PID server.PID

reboot: stop clean build start

registry.PID:
	@echo
	@echo STARTING RMI REGISTRY
	cd build && { rmiregistry & echo $$! > ../$@; }
	@sleep 2
	@echo

server.PID: registry.PID
	@echo STARTING PRIORITY SERVER
	java -classpath build -Djava.rmi.server.codebase=file:build/ Server & echo $$! > $@
	@sleep 2
	@echo

client: server.PID
	java -classpath build Client $(fk) $(ct)
	@sleep 2

stop:
	@echo KILLING RMI REGISTRY
	kill `cat registry.PID` && rm registry.PID
	@echo
	
	@echo KILLING RMI SERVER
	kill `cat server.PID` && rm server.PID
	@echo

build: Client.java Resource.java Server.java
	mkdir build
	javac -d build Client.java Resource.java Server.java

clean:
	rm -R build

test:
	make client fk=1 &
	make client fk=1 ct=pepe1 &
	make client fk=1 ct=pepe2 &
	make client fk=1 ct=pepe3 &
	make client fk=1 &


.PHONY: run start stop build clean client