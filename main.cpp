//
//  main.cpp
//  mp4
//
//  Created by Mitesh Patel on 10/6/16.
//  Copyright (c) 2016 TAMU. All rights reserved.
//

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

string validString = "";
bool myPipe = false;
bool special = false;
bool unixC = false;
bool uAmp = false;
bool redirection = false;

bool isUnixCommand(string command, int stat);
bool isSingleRedirect(vector<string>* commands);
vector<string> tokenizer(string input);

void callUnixExec(string valid) {
	vector<string> args = tokenizer(valid);
	vector<char*> cstrings;
	for (int i = 0; i < args.size(); ++i)
		cstrings.push_back(const_cast<char*>(args[i].c_str()));
	cstrings.push_back(NULL);
	char** prog = &cstrings[0];
	int status = execvp(prog[0], prog);
	validString.clear();
	return;
}

void callSpecialExec(string valid) {
	vector<string> args = tokenizer(valid);
	vector<char*> cstrings;
	for (int i = 0; i < args.size(); ++i)
		cstrings.push_back(const_cast<char*>(args[i].c_str()));
	cstrings.push_back(NULL);
	char* dir = cstrings[1];
	int status = chdir(dir);
	validString.clear();
	return;
}
/*

void callPipeExec(string valid);
void callRedExec(string valid);
void callAmpExec(string valid);
*/
// Tokenizes the string for parsing
vector<string> tokenizer(string input) {
	int temp = 0;
	vector<string> out;
	while (temp < input.size()) {
		
		string tempStr = "";
		while (input[temp] != ' ' && input[temp] != '\"' && input[temp] != '|'
			&& input[temp] != '<' && input[temp] != '>' && input[temp] != '&' && temp < input.size()) {
			tempStr += input[temp];
			temp++;
		}
		if (input[temp] == ' ') {
			temp++;
		}
		else if (input[temp] == '\"') {
			tempStr = input.substr(temp, (input.substr(temp+1)).find("\"")+2);
			temp = temp + 2 + (input.substr(temp+1)).find("\"");
		}
		else if (input[temp] == '|') {
			if (tempStr != "")
				out.push_back(tempStr);
			tempStr.clear();
			temp++;
			tempStr = "|";
		}
		else if (input[temp] == '<') {
			if (tempStr != "")
				out.push_back(tempStr);
			tempStr.clear();
			temp++;
			tempStr = "<";
		}
		else if (input[temp] == '>') {
			if (tempStr != "")
				out.push_back(tempStr);
			tempStr.clear();
			temp++;
			tempStr = ">";
		}
		else if (input[temp] == '&') {
			if (tempStr != "")
				out.push_back(tempStr);
			tempStr.clear();
			temp++;
			tempStr = "&";
		}
		if (tempStr != "")
			out.push_back(tempStr);
	}
	return out;
}

// Initial separation by command
// Syntax check implemented in another function
vector<string> commandTokenizer(vector<string> tokens) {
	vector<string> commands;
	string command = "";
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] != "|" && tokens[i] != "<" && tokens[i] != ">" && tokens[i] != "&") {
			command += tokens[i] + " ";
			tokens.erase(tokens.begin() + i);
			i--;
		} 
		else {
			if (!command.empty()) {
				commands.push_back(command);
				command = "";
			}
			commands.push_back(tokens[i]);
			tokens.erase(tokens.begin() + i);
			i--;
		}
		if (i == tokens.size() - 1) {
			if (!command.empty()) {
				commands.push_back(command);
				command = "";
			}
			tokens.clear();
			break;
		}
	}
	return commands;
}

bool isPipe(vector<string>* commands) {
	if (commands->size() < 3)
		return false;
	bool first = isUnixCommand((*commands)[0], 0);
	bool second = (*commands)[1] == "|";
	if (first && second)
		validString += (*commands)[0] + (*commands)[1] + " ";
	else {
		return false;
	}
	commands->erase(commands->begin(), commands->begin() + 2);
	bool third = false;
	if (isPipe(commands)) {
		third = true;
	}
	else if (commands->size() == 3) {
		if (isSingleRedirect(commands))
			third = true;
	}
	else if (isUnixCommand((*commands)[0], 0)) {
		third = true;
	}
	if (third) {
		if (commands->size() >= 1) {
			validString += (*commands)[0];
			commands->erase(commands->begin());
		}
	}
	return first && second && third && commands->empty();
}

bool isUnixCommand(string command, int stat) {
	if (command.find("<") == string::npos) {
		if (command.find(">") == string::npos) {
			if (command.find("|") == string::npos) {
				if (command.find("&") == string::npos) {
					if (command != "") {
						if (stat == 1)
							validString += command;
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool isRedirection(vector<string>* commands) {
	if (commands->size() < 3)
		return false;
	bool first = isUnixCommand((*commands)[0], 0);
	bool second = (*commands)[1] == "<" || (*commands)[1] == ">";
	if (first && second)
		validString += (*commands)[0] + (*commands)[1] + " ";
	else {
		return false;
	}
	commands->erase(commands->begin(), commands->begin() + 2);
	bool third = false;
	if (isRedirection(commands)) {
		third = true;
	}
	else if (isUnixCommand((*commands)[0], 0)) {
		third = true;
	}
	if (third) {
		if (commands->size() >= 1) {
			validString += (*commands)[0];
			commands->erase(commands->begin());
		}
	}
	return first && second && third && commands->empty();
}

bool isSingleRedirect(vector<string>* commands) {
	if (commands->size() < 3)
		return false;
	bool first = isUnixCommand((*commands)[0], 0);
	bool second = (*commands)[1] == ">";
	if (first && second)
		validString += (*commands)[0] + (*commands)[1] + " ";
	else {
		return false;
	}
	commands->erase(commands->begin(), commands->begin() + 2);
	bool third = false;
	if (isUnixCommand((*commands)[0], 0)) {
		third = true;
	}
	if (third) {
		if (commands->size() >= 1) {
			validString += (*commands)[0];
			commands->erase(commands->begin());
		}
	}
	return first && second && third && commands->empty();
}

bool isAMP(vector<string> commands) {
	if (commands.size() < 2)
		return false;
	bool first = isUnixCommand(commands[0], 0);
	bool second = commands[1] == "&";
	if (first && second) {
		validString += commands[0] + commands[1];
		commands.erase(commands.begin(), commands.begin() + 2);
	}
	
	return first && second;
}

bool isSpecial(string command) {
	if (isUnixCommand(command, 0)) {
		if (command.substr(0, 2) == "cd") {
			validString += command;
			return true;
		}
	}
	return false;
}


bool isValidString(vector<string> commands) {
	validString.clear();
	vector<string> commands_one = commands;
	if (commands.size() == 1) {
		if (isSpecial(commands[0])) {
			special = true;
			commands.erase(commands.begin());
			return special;
		}
		validString.clear();
		unixC = isUnixCommand(commands[0], 1);
		if (!unixC)
			validString.clear();
		commands.erase(commands.begin());
		return unixC;
	}

	if (isAMP(commands)) {
		uAmp = true;
		return uAmp;
	}
	validString.clear();
	if (isPipe(&commands)) {
		myPipe = true;
		return myPipe;
	}
	validString.clear();
	if (isRedirection(&commands_one)) {
		redirection = true;
		return redirection;
	}
	validString.clear();
	return false;
}

int main(int argc, const char * argv[]) {
    
	while (true){
		//fork and wait
		string input = "";
		cout << "mp4: $ ";
		getline(cin, input);
		if (input == "exit")
			return 0;
		pid_t pid = fork();
		if (pid == 0) {
			vector<string> commands = commandTokenizer(tokenizer(input));
			isValidString(commands);
			if (unixC) {
				//execute unix command
				callUnixExec(validString);
				return 0;
			}

			else if (special) {
				//execute special command
				callSpecialExec(validString);
				//return 0;
			}
			else {
				cout << "INVALID SYNTAX" << endl;
			}
			/*
			if (myPipe) {
				//execute pipe instruction
				callPipeExec(validString);
			}
			else if (redirection) {
				//execute redirection instruction
				callRedExec(validString);
			}
			else if (uAmp) {
				//execute AMP instruction
				callAmpExec(validString);
			}
			*/
			unixC = false;
			special = false;
			uAmp = false;
			redirection = false;
			myPipe = false;
		}
		else if (pid == -1) {
			cout << "ERROR: " << endl;
		}
		else {
			waitpid(pid, NULL, 0);
		}
		
    /*
    if (validate(input)){
			- find out what commands input is wanting to do
      - execute the command using fork() and exec()
    }
    */
	}
    
    
	return 0;
}


/** Description: parses the input according to grammer provided
 *  and validates the input. Parse using a c++ lib.
 *
 *
 *@param String
 *@return bool
 **/
bool validate (string input){
	return true;
}
