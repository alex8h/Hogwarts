#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include <string>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include <iostream>

using namespace std;

/*
Hogwarts Houses:
	- Gryffindor (G)
	- Hufflepuff (H)
	- Ravenclaw  (R)
	- Slytherin  (S)
*/

/*
All rights:
	- SELECT
	- INSERT
    - UPDATE
    - DELETE
	- CREATE
    - ALTER
    - DROP
*/

char getch()
{
	char ch;
	struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

// Улучшением было бы добавить базу данных пользователей.

struct hogwarts_director
{
	string name;
	string pass;
	string rights = "allrights";
};

struct professors
{
	string name;
	string pass;
	string rights = "SELECT,INSERT,UPDATE,DELETE";
	string house;	// may be empty
};

struct students
{
	string name;
	string pass;
	string rights = {"SELECT"};
	string house;
};

template < typename T >
int get_index(T type, string name, int n)
{
	for(int i = 0; i < n; i++)
	{
		if(type[i].name == name)
			return i;
	}
	return -1;
}

bool find_right(professors prof, string str)
{
	if(prof.rights.find(str) == -1)
		return 0;
	return 1;
}


int callback(void *data, int argc, char **argv, char **azColName)
{
	for(int i = 0; i < argc; i++)
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   	printf("\n");
   	return 0;
}

int main()
{
	int number = 10;
	hogwarts_director director[1];
	professors professor[number];
	students student[number];

	director[0].name = "Dumbledore";
	director[0].pass = "beard";
	professor[0].name = "McGonagall";
	professor[0].pass = "nose"; 
	professor[0].house = "G";
	student[0].name = "H.Potter";
	student[0].pass = "scar";
	student[0].house = "G";



/////////////// аутентификация





	string name;
	char type;
	int index;
	cout<<"Введите тип пользователя"<<endl;
	cin>>type;
	cout<<"Введите имя пользователя"<<endl;
	getline(cin,name);
	
	//password = "nose";
	//type = 'p';
	//name = "McGonagall";



	cout<<"Введите пароль"<<endl;
	char line;
	string password;
	for(int i = 0; line = getch(); i++) 
	{
    	if (line == '\r' || line == ' ' || line == '\t' || line == '\n')
    	{
        	//password += '\0';
        	break;
		}
		password += line;
    	cout<<"*";
	}
	cout<<endl;


	switch (type)
	{
		case 'd':
			index = get_index(director,name,1);
			if(index == -1)
			{
				cout<<"Wrong type or name"<<endl;
				return 0;
			}
			if(director[index].pass != password)
			{
				//cout<<(director[index].pass).length()<<endl;
				//cout<<(password).length()<<endl;
				cout<<"Wrong password"<<endl;
				return 0;
			}
			break;
		
		case 'p':
		{	
			index = get_index(professor,name,number);
			if(index == -1)
			{
				cout<<"Wrong type or name"<<endl;
				return 0;
			}
			if(professor[index].pass != password)
			{
				cout<<"Wrong password"<<endl;
				return 0;
			}
			break;
		}
		case 's':
		{	
			index = get_index(student,name,number);
			if(index == -1)
			{
				cout<<"Wrong type or name"<<endl;
				return 0;
			}
			if(student[index].pass != password)
			{
				cout<<"Wrong password"<<endl;
				return 0;
			}
			break;
		}
		default: 
			cout<<"Wrong type"<<endl;
			return 0;
	}

	cout<<"Авторизация пройдена успешно"<<endl;


 
	sqlite3 *db = 0; // хэндл объекта соединение к БД
	char *err = 0;
	 const char* data = "";//"Callback function called";

	cout<<"Открытие базы данных..."<<endl;

	if( sqlite3_open("hogwarts.db", &db) )
		cout<<stderr<<" "<<"Ошибка открытия/создания БД."<<" "<<sqlite3_errmsg(db)<<endl;
	else
	{
		cout<<"База данных готова к командам(вводить по одной)."<<endl;		
		
		while(true)
		{
			string SQL = "\0";
			//SQL = "";
			getline(cin,SQL);
			//SQL = "DROP TABLE qwer;";
			cout<<SQL<<endl;
			if(SQL == "stop")
			{
				sqlite3_close(db);
				return 0;
			}

			int flag = SQL.find(";");
			SQL = SQL.substr(0,flag+1);
			char * cstr = new char [SQL.length()+1];
  			strcpy(cstr, SQL.c_str());

			if(type == 'd')
			{
				if(sqlite3_exec(db, cstr, callback, (void*)data, &err))
				{
					cout<<stderr<<"  Ошибка SQL: "<<err<<endl;
					sqlite3_free(err);
					continue;
				}
				continue;
			}
			else if(type == 's' && SQL.substr(0,6) == "SELECT") 
			{
				if(sqlite3_exec(db, cstr, callback, (void*)data, &err))
				{
					cout<<stderr<<"  Ошибка SQL: "<<err<<endl;
					sqlite3_free(err);
					continue;
				}
				continue;
			}
			else if(type == 'p' && find_right(professor[index], SQL.substr(0,6)))
			{
				if(sqlite3_exec(db, cstr, callback, (void*)data, &err))
				{
					cout<<stderr<<"  Ошибка SQL: "<<err<<endl;
					sqlite3_free(err);
					continue;
				}
				continue;

			}
			else
				cout<<"Недостаточно прав для данной команды"<<endl;

		}	
	}





	// закрываем соединение
	sqlite3_close(db);
	return 0;
}