all: build																															
build: 																															
	@echo "Building" 																										
	gcc -o output shell.c 																		
	./output												
clean:                                                                 
	@echo "cleaning" 
	rm output