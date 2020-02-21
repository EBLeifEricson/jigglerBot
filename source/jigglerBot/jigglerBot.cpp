// jigglerBot, a basic Twitch chat bot for speedrunners by LeifEricson

#include "libircclient.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <thread>
#include <sstream>
#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <curl/curl.h>
using namespace std;

queue<string> commandQueue; // Queue to store commands during cooldown
map<string, string> customCommands;
vector<string> globalQuotes;
unordered_map<string, string> categories;
string globalChannel; // Channel to speak in
string globalUsername;
string globalAuthKey;
string motd;
string globalGame;
string actualName;
string versionString = "v2.0.1-beta";
bool globalMute;
bool automotd;
int globalCooldown;

struct write_result {
	char *data;
	int pos;
};

string lowercaseStr(string input) {
	for (char& c : input) {
		c = tolower(c);
	}
	return input;
}

size_t curl_write(void *ptr, size_t size, size_t nmemb, void *stream) {

	struct write_result *result = (struct write_result *)stream;

	/* Will we overflow on this write? */
	if (result->pos + size * nmemb >= (256 * 1024) - 1) {
		fprintf(stderr, "curl error: too small buffer\n");
		return 0;
	}

	/* Copy curl's stream buffer into our own buffer */
	memcpy(result->data + result->pos, ptr, size * nmemb);

	/* Advance the position */
	result->pos += size * nmemb;

	return size * nmemb;
}

bool fileExists(const string& filename) {
	ifstream ifs(filename);
	return (bool)ifs;
}

void delCommand(const string& command) {
	map<string, string>::iterator item = customCommands.find(command);
	if (item != customCommands.end()) {
		customCommands.erase(item->first);
	}
	ofstream ofs("commands.txt");
	for (map<string, string>::iterator itr = customCommands.begin(); itr != customCommands.end(); ++itr) {
		ofs << itr->first << " " << itr->second << endl;
	}
	ofs.close();
}

void loadCommands(const string& filename) {
	customCommands.empty();
	ifstream ifs(filename);
	if (!ifs) return;
	string command;
	string result;
	while (ifs >> command) {
		ifs.seekg(1, ifs.cur);
		getline(ifs, result);
		customCommands[command] = result;
	}
	ifs.close();
}

void loadQuotes() {
	globalQuotes.empty();
	ifstream ifs("quotes.txt");
	if (!ifs) return;
	string result;
	while (getline(ifs, result)) {
		globalQuotes.push_back(result);
	}
	ifs.close();
}

void loadConfig(const string& filename) {
	string discard;
	if (!fileExists(filename)) {
		ofstream ofs(filename);
		ofs << "username: botusername\noauthkey: bot_oauth_key_goes_here\nchannel: yourchannelhere\nmotd: Please type a message of the day here.\nstartMuted: 0\nautoMOTD: 1\ncooldown: 1500";
		ofs.close();
	}
	ifstream ifs(filename);
	ifs >> discard >> globalUsername >> discard >> globalAuthKey >> discard >> globalChannel >> discard;
	globalChannel = "#" + globalChannel;
	getline(ifs, motd);
	ifs >> discard >> discard;
	globalMute = atoi(discard.c_str());
	ifs >> discard >> discard;
	automotd = atoi(discard.c_str());
	ifs >> discard >> discard;
	globalCooldown = atoi(discard.c_str());
	ifs.close();
}

void delMod(const string& name) {
	ifstream ifs("mods.txt");
	if (!ifs) return;
	vector<string> mods;
	string mod;
	while (getline(ifs, mod)) {
		if (mod != name) mods.push_back(mod);
	}
	ifs.close();
	ofstream ofs("mods.txt");
	for (string user : mods) {
		ofs << user << endl;
	}
	ofs.close();
}

void addMod(const string& modname) {
	ifstream test("mods.txt");
	ofstream ofs;
	if (test) {
		test.close();
		ofs = ofstream("mods.txt", std::ios::in | std::ios::out | std::ios::ate);
	}
	else {
		test.close();
		ofs = ofstream("mods.txt");
	}
	ofs << modname << endl;
	ofs.close();
}

int addQuote(const string& quoteToAdd) {
	ifstream quoteIn("quotes.txt");
	ofstream quoteOut;
	int openquote = 0;
	bool foundOpen = false;
	if (quoteIn) {
		vector<string> quotes;
		string quote;
		while (getline(quoteIn, quote)) {
			if (quote == "null$_!null" && !foundOpen) {
				foundOpen = true;
				quotes.push_back(quoteToAdd);
			}
			else quotes.push_back(quote);
			if (!foundOpen) ++openquote;
		}
		if (!foundOpen) quotes.push_back(quoteToAdd);
		quoteIn.close();
		quoteOut = ofstream("quotes.txt", std::ios::in | std::ios::out);
		for (string quote : quotes) {
			quoteOut << quote << endl;
		}
		globalQuotes = quotes;
	}
	else {
		quoteIn.close();
		quoteOut = ofstream("quotes.txt");
		globalQuotes.empty();
	}
	quoteOut.close();
	return openquote;
}

void delQuote(const int& line) {
	ifstream ifs("quotes.txt");
	if (!ifs) return;
	vector<string> quotes;
	string quote;
	int linenumber = 0;
	while (getline(ifs, quote)) {
		if (linenumber == line) quotes.push_back("null$_!null");
		else quotes.push_back(quote);
		++linenumber;
	}
	ifs.close();
	ofstream ofs("quotes.txt");
	for (string quote : quotes) {
		ofs << quote << endl;
	}
	ofs.close();
	globalQuotes = quotes;
}

bool isMod(const string& user) {
	// Owner override, I use this in local builds but would be kind of sneaky to put this in a release build...
	// if (user == "leifericson") return true;
	
	// Load moderators from file
	ifstream ifs("mods.txt");
	if (!ifs) return false;
	string mod;
	while (getline(ifs, mod)) {
		if (mod == user) {
			ifs.close();
			return true;
		}
	}
	ifs.close();
	return false;
}

// Change the color of the console output, valid inputs are 0-15
void SetColor(const int& value) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), value);
}

// Threaded function to display the message of the day, once every 20 minutes
void printMOTD(irc_session_t * session) {
	while (true) {
		if (!globalMute && automotd) irc_cmd_msg(session, globalChannel.c_str(), motd.c_str());
		Sleep(3600000);
	}
}

// Threaded function that always empties the command queue
void handleQueue(irc_session_t * session) {
	while (true) {
		// If queue not empty
		if (commandQueue.size() > 0) {
			// Say the message on the queue if bot isn't muted
			if (!globalMute) irc_cmd_msg(session, globalChannel.c_str(), commandQueue.front().c_str());
			// Pop the message off of the queue
			commandQueue.pop();
		}
		Sleep(globalCooldown); // Cooldown
	}
}

// Function called when connecting to channel (execute any start up code here)
void event_connect(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count) {
	cerr << "Got connect event." << endl;
	// Try to join the channel
	if (irc_cmd_join(session, globalChannel.c_str(), 0)) {
		cerr << "Error: Failed to join channel." << endl;
		exit(-1);
	}
	cerr << "Connected to channel " << string(globalChannel) << endl;

	// Start and detatch the background monitor functions
	thread queueMonitor(handleQueue, session);
	thread motdMonitor(printMOTD, session);
	queueMonitor.detach();
	motdMonitor.detach();
}

// This is needed for certain IRC events, but not actually used in this program so it's been stubbed
void event_numeric(irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count) {}

// Called whenever someone says something. Use this to test for commands.
void event_channel(irc_session_t * session, const char * event, const char * origin,
	const char ** params, unsigned int count) {
	int index = string(origin).find("!");
	string user = lowercaseStr(string(origin).substr(0, index));
	string command = lowercaseStr(string(params[1]));
	int color = ((int(origin[0]) + int(origin[2]) + int(origin[3])) % 14) + 1;
	if (color == 1) color = 3;
	SetColor(color);
	cout << user;
	SetColor(15);
	cout << ": " << params[1] << endl;

	auto t = time(nullptr);
	auto tm = *localtime(&t);
	ofstream ofs("log.txt", std::ios::in | std::ios::out | std::ios::ate);
	ofs << put_time(&tm, "[%H:%M:%S] ") << user << ": " << command << endl;

	if (params[1][0] == '!') {
		SetColor(12);
		cout << "CONSOLE: Command detected: " << command << endl;
		ofs << "CONSOLE: Command detected: " << command << endl;
		SetColor(15);
		// ADMIN COMMANDS
		if (command == "!test") {
			if (isMod(user)) commandQueue.push("Testing 123...");
		}
		else if ((command == "!quit" || command == "!exit")) {
			if (isMod(user)) {
				irc_cmd_msg(session, globalChannel.c_str(), "Beep. Boop. Shutting down...");
				Sleep(3000);
				exit(1);
			}
		}
		else if (command == "!motd") {
			if (isMod(user)) commandQueue.push(motd);
		}
		else if (command == "!mutebot") {
			if (isMod(user)) {
				if (globalMute == 1) {
					globalMute = 0;
					irc_cmd_msg(session, globalChannel.c_str(), "Beep. Boop. JigglerBot has been unmuted!");
					commandQueue.empty();
				}
				else {
					irc_cmd_msg(session, globalChannel.c_str(), "Beep. Boop. JigglerBot has been muted...");
					globalMute = 1;
				}
			}
		}
		else if (command.substr(0, 7) == "!addmod") {
			if (isMod(user)) {
				string modname = command.substr(8, command.size() - 8);
				if (!isMod(modname)) {
					addMod(modname);
					commandQueue.push("Allowed " + modname + " access to admin commands!");
				}
			}
		}
 		else if (command.substr(0, 7) == "!delmod") {
			if (isMod(user)) {
				string modname = command.substr(8, command.size() - 8);
				if (isMod(modname)) {
					delMod(modname);
					commandQueue.push("Removed " + modname + "'s access to admin commands!");
				}
			}
		} 
		else if (command.substr(0, 7) == "!addcom") {
			if (isMod(user)) {
				int offset = command.find(" ", 8);
				string newComm = command.substr(8, offset-8);
				string newVal = command.substr(offset+1, command.size() - offset - 1);
				ofstream ofs;
				if (fileExists("commands.txt")) ofs.open("commands.txt", std::ios::in | std::ios::out | std::ios::ate);
				else ofs.open("commands.txt");
				ofs << newComm << " " << newVal << endl;
				ofs.close();
				loadCommands("commands.txt");
				commandQueue.push("Added command \"!" + newComm + ".\"");
			}
		}
		else if (command.substr(0, 7) == "!delcom") {
			if (isMod(user)) {
				int offset = command.find(" ", 8);
				string newComm = command.substr(8, command.size() - 8);
				delCommand(newComm);
				loadCommands("commands.txt");
				commandQueue.push("Removed command \"!" + newComm + ".\"");
			}
		}
		else if (command == "!reload") {
			if (isMod(user)) {
				loadCommands("commands.txt");
				loadConfig("config.txt");
				commandQueue.push("Reloaded configuration and commands.");
			}
		}

		else if (command.substr(0, 6) == "!quote") {
			if (isMod(user) && (command.size() > 10) && (command.substr(7, 3) == "add")) {
				int line = addQuote(string(params[1]).substr(11, string(params[1]).size() - 11));
				commandQueue.push("Added quote #"+to_string(line+1)+": "+ string(params[1]).substr(11, command.size() - 11)+"");
			}
			else if (isMod(user) && (command.size() > 13) && (command.substr(7, 6) == "remove")) {
				int quoteToRemove = atoi(string(params[1]).substr(14, string(params[1]).size() - 14).c_str())-1;
				delQuote(quoteToRemove);
				commandQueue.push("Removed quote #" + to_string(quoteToRemove+1));
			}
			else if (command.size() > 7) {
				int quoteNum = atoi(command.substr(7, command.size() - 7).c_str()) - 1;
				if (quoteNum < globalQuotes.size() && globalQuotes[quoteNum] != "null$_!null") commandQueue.push(globalQuotes[quoteNum]);
				else commandQueue.push("Quote not found!");
			}
			else {
				if (globalQuotes.size() > 0) commandQueue.push(globalQuotes[rand() % globalQuotes.size()]);
				else commandQueue.push("No quotes added!");
			}
		}

		else {
			map<string, string>::iterator item = customCommands.find(command.substr(1, command.size() - 1));
			if (item != customCommands.end()) commandQueue.push(item->second);
		}
	}
	ofs.close();
}

int main() {
	cerr << "JigglerBot " << versionString << " by LeifEricson\nRecovery of old source code with some functions missing.\n\n";
	printf("Begin setup...\n");
	loadConfig("config.txt");
	loadCommands("commands.txt");
	loadQuotes();
	irc_callbacks_t callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.event_connect = event_connect;
	callbacks.event_numeric = event_numeric;
	callbacks.event_channel = event_channel;
	irc_session_t *session = irc_create_session(&callbacks);
	if (irc_connect(session, "irc.twitch.tv", 6667, globalAuthKey.c_str(), globalUsername.c_str(), globalUsername.c_str(), globalUsername.c_str())) {
		return 1;
	}
	if (irc_run(session)) {
	}
	return 0;
}