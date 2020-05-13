make:
	#rm hogwarts.db
	g++ main.cpp -o main -lsqlite3
	./main
