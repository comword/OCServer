all:
	rm -rf bin
	mkdir bin
	javac -d bin $$(find src/org -name "*.java")
	cd bin && jar cvf ../JavaP.jar *
clean:
	rm -rf bin
	rm -rf JavaP.jar
