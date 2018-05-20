/*
	Copyright (C) 2014 CurlyMo & wo_rasp

	This file is part of pilight.

	pilight is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version.

	pilight is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with pilight. If not, see	<http://www.gnu.org/licenses/>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../../core/pilight.h"
#include "../../core/common.h"
#include "../../core/dso.h"
#include "../../core/log.h"
#include "../protocol.h"
#include "../../core/binary.h"
#include "../../core/gc.h"
#include "program_screen.h"


void *execute(void *cmd) {
	system((char*) cmd);
}

static void createMessage(char *name, int state, char *up_cmd, char *down_cmd) {
	program_screen->message = json_mkobject();
	json_append_member(program_screen->message, "name", json_mkstring(name));
	json_append_member(program_screen->message, "up-command", json_mkstring(up_cmd));
	json_append_member(program_screen->message, "down-command", json_mkstring(down_cmd));

	if(state==0) {
		json_append_member(program_screen->message, "state", json_mkstring("up"));
	} else {
		json_append_member(program_screen->message, "state", json_mkstring("down"));
	}
}

static int createCode(JsonNode *code) {
	double itmp = -1;
	int    state = -1;
   char   *up_cmd = NULL, *down_cmd = NULL, *name = NULL;

	if(json_find_number(code, "up", &itmp) == 0)
		state=0;
	if(json_find_number(code, "down", &itmp) == 0)
		state=1;

	if(state==-1) {
		logprintf(LOG_ERR, "program_screen: insufficient number of arguments: missing state");
		return EXIT_FAILURE;
	}

   if (json_find_string(code, "up-command", &up_cmd)) {
      logprintf(LOG_ERR, "program_screen: insufficient number of arguments: missing up-command");
      return EXIT_FAILURE;
   }

   if (json_find_string(code, "down-command", &down_cmd)) {
      logprintf(LOG_ERR, "program_screen: insufficient number of arguments: missing down-command");
      return EXIT_FAILURE;
   }

   if (json_find_string(code, "name", &name)) {
      logprintf(LOG_ERR, "program_screen: insufficient number of arguments: missing name");
      return EXIT_FAILURE;
   }

	createMessage(name, state, up_cmd, down_cmd);

	char *cmd;
   if(state == 0) {
      system(up_cmd);
   }
   else if(state == 1) {
      system(down_cmd);
   }

   return EXIT_SUCCESS;
}

static void printHelp(void) {
	printf("\t -u --up-command=command\t\t\tcommand that is executed on up\n");
	printf("\t -d --down-command=command\t\t\tcommand that is executed on down\n");
	printf("\t -f --up\t\t\tsend an darken DOWN command to the selected device\n");
	printf("\t -t --down\t\t\tsend an brighten UP command to the selected device\n");
}

#if !defined(MODULE) && !defined(_WIN32)
__attribute__((weak))
#endif
void programScreenInit(void) {

	protocol_register(&program_screen);
	protocol_set_id(program_screen, "program_screen");
	protocol_device_add(program_screen, "program_screen", "Program Screen");
	program_screen->devtype = SCREEN;
	program_screen->hwtype = RF433;

	options_add(&program_screen->options, 'n', "name", OPTION_NO_VALUE, DEVICES_ID, JSON_STRING, NULL, NULL);
	options_add(&program_screen->options, 't', "up", OPTION_NO_VALUE, DEVICES_STATE, JSON_STRING, NULL, NULL);
	options_add(&program_screen->options, 'f', "down", OPTION_NO_VALUE, DEVICES_STATE, JSON_STRING, NULL, NULL);
	options_add(&program_screen->options, 'u', "up-command", OPTION_HAS_VALUE, DEVICES_SETTING, JSON_STRING, NULL, NULL);
	options_add(&program_screen->options, 'c', "down-command", OPTION_HAS_VALUE, DEVICES_SETTING, JSON_STRING, NULL, NULL);

	options_add(&program_screen->options, 0, "readonly", OPTION_HAS_VALUE, GUI_SETTING, JSON_NUMBER, (void *)0, "^[10]{1}$");
	options_add(&program_screen->options, 0, "confirm", OPTION_HAS_VALUE, GUI_SETTING, JSON_NUMBER, (void *)0, "^[10]{1}$");

	program_screen->createCode=&createCode;
	program_screen->printHelp=&printHelp;
}

#if defined(MODULE) && !defined(_WIN32)
void compatibility(struct module_t *module) {
	module->name = "program_screen";
	module->version = "2.2";
	module->reqversion = "6.0";
	module->reqcommit = "84";
}

void init(void) {
	programScreenInit();
}
#endif
