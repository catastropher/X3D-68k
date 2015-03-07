// C Source File
// Created 3/5/2015; 3:48:47 PM

#include "console.h"

#include <tigcclib.h>

#define LINE_LENGTH 70
#define MAX_LINES 15

typedef struct ConsoleLine {
	short cat;
	char line[LINE_LENGTH];
	char desc[30];
} ConsoleLine;

ConsoleLine* console_line;

int console_line_head;
int console_line_tail;

void init_console() {
	console_line_head = 0;
	console_line_tail = 0;
	console_line = malloc(sizeof(ConsoleLine) * MAX_LINES);
}

short console_total_lines() {
	if(console_line_tail > console_line_head)
		return console_line_tail - console_line_head;
	else
		return MAX_LINES + console_line_tail - console_line_head;
}

const char* console_cat_name[] = {
	"ERROR",
	"--->",
	"CLIP",
	"CMD"
};

short get_console_key(char* is_edit) {
	unsigned short key = ngetchx();
	
	*is_edit = 1;
	
	if(isdigit(key) || isalpha(key) || key == ' ' || key == '(' || key == ')' || key == '=' || key == '.' ||
		key == '"' || key == ',' || key == '+')
			return key;
	
	*is_edit = 0;
	
	return key;
}

enum {
	TYPE_CHAR,
	TYPE_INT,
	TYPE_LONG,
	TYPE_STRING,
	TYPE_VEX3D,
	TYPE_VEX2D,
	TYPE_PLANE,
	TYPE_CMD
};

#define VAR_SIZE 64

typedef struct {
	short size;
	short type;
	unsigned char* data;
} Value;

typedef struct {
	char name[9];
	char in_use;
	char is_extern;
	char should_free;
	void* mem;
	
	Value val;
	
	char keep_type;
	
} AllocBlock;

typedef AllocBlock Var;

typedef struct {
	short capacity;
	AllocBlock** block;
} Allocator;

Allocator alloc;

typedef struct {
	short min_argc;
	short max_argc;
	const char* name;
	char (*handler)(char* line, Value* args, int argc, Value* dest);
} CMDHandler;


char cmd_print(char* line, Value* args, int argc, Value* dest) {
	int i;
	
	for(i = 0; i < argc; i++) {
		if(args[i].type == TYPE_STRING)
			cprintf(PRINT, "%s", args[i].data);
		else if(args[i].type == TYPE_INT) {
			cprintf(PRINT, "%d", *((short *)args[i].data));
		}
		else if(args[i].type == TYPE_LONG) {
			cprintf(PRINT, "%ld", *((long *)args[i].data));
		}
	}

	return 1;
}

long value_as_long(Value* v) {
	if(v->type == TYPE_CHAR)
		return *((char *)v->data);
	else if(v->type == TYPE_INT)
		return *((short *)v->data);
	else
		return *((long *)v->data);
}

void long_as_value(Value* v, long val, short type) {
	if(type == TYPE_CHAR)
		*((char *)v->data) = val;
	else if(type == TYPE_INT)
		*((short *)v->data) = (short)val;
	else if(type == TYPE_LONG)
		*((long *)v->data) = val;
		
	v->type = type;
}

char cmd_add(char* line, Value* args, int argc, Value* dest) {
	long res = 0;
	char type = TYPE_CHAR;
	int i;
	
	for(i = 0; i < argc; i++) {
		if(args[i].type > TYPE_LONG) {
			cprintf(ERROR, "Type error -> %s", line);
			return 0;
		}
		
		res += value_as_long(&args[i]);
		type = max(type, args[i].type);
	}
	
	long_as_value(dest, res, type);
	
	return 1;
	
}

CMDHandler cmd_tab[] = {
	{
		1, 100, "print", cmd_print
	},
	{
		1, 100, "add", cmd_add
	}
};



void* alloc_block() {
	int i;
	
	for(i = 0;i < alloc.capacity; i++) {
		if(!alloc.block[i]->in_use) {
			alloc.block[i]->in_use = 1;
			return alloc.block[i]->mem;
		}
	}
	
	alloc.capacity += 5;
	realloc(alloc.block, alloc.capacity * sizeof(AllocBlock));
	
	void* new_mem = calloc(5, VAR_SIZE);
	
	alloc.block[alloc.capacity - 5]->in_use = 1;
	alloc.block[alloc.capacity - 5]->should_free = 1;
	alloc.block[alloc.capacity - 5]->mem = new_mem;
	
	for(i = alloc.capacity - 4; i < alloc.capacity; i++) {
		new_mem += VAR_SIZE;
		
		alloc.block[i]->in_use = 0;
		alloc.block[i]->should_free = 0;
		alloc.block[i]->mem = new_mem;
	}
	
	return alloc.block[alloc.capacity - 5];
}

#define INVALID_VAR -1

short find_var(char* name) {
	int i;
	
	for(i = 0; i < alloc.capacity; i++) {
		if(alloc.block[i]->in_use && strcmp(alloc.block[i]->name, name) == 0) {
			return i;
		}
	}
	
	return INVALID_VAR;
}

void copy_val(Value* src, Value* dest) {
	memcpy(dest->data, src->data, src->size);
	dest->size = src->size;
	dest->type = src->type;
}

Value* get_var(short id) {
	return &alloc.block[id]->val;
}

#define TYPE_ERROR -1

char set_var(short id, Value* v) {
	Var* var = alloc.block[id];
	
	if(var->keep_type && var->val.type != v->type) {
		return TYPE_ERROR;
	}
	else {
		copy_val(v, &var->val);
		return 1;
	}
}

typedef struct {
	Value val;
	unsigned char data[VAR_SIZE];
} Temp;

char execute_function(char* line, char* name, Value* dest, Value args[], int argc) {
	int i;
	int total_cmd = sizeof(cmd_tab) / sizeof(CMDHandler);
	
	for(i = 0; i < total_cmd; i++) {
		if(strcmp(cmd_tab[i].name, name) == 0) {
			return cmd_tab[i].handler(line, args, argc, dest);
		}
	}
	
	cprintf(ERROR, "Invalid function %s -> %s", name, line);
	return 0;
}

void consume_whitespace(char** start, char* end) {
	while(*start < end && **start == ' ') {
		(*start)++;
	}
}

char is_valid_func_char(char c) {
	return isdigit(c) || isalpha(c);
}

char is_valid_digit(char c) {
	return isdigit(c);
}

char is_valid_op(char c) {
	return c == '+' || c == '-' || c == '/' || c == '*' || c == '=';
}

char* get_token(char* token, char* start, char* end, char(*is_valid)(char)) {
	short token_pos = 0;
	
	while(start < end && is_valid(*start)) {
		token[token_pos++] = *start;
		start++;
	}
	
	token[token_pos] = '\0';
	
	return start;
}

enum {
	TOKEN_NONE,
	TOKEN_NUM,
	TOKEN_ID,
	TOKEN_STRING,
	TOKEN_PAR,
	TOKEN_OP
};

#define SWAP(_a,_b) {typeof(_a) _save = _a; _a = _b; _b = _save;}

char execute_statement(char* line, char* start, char *end, Value* res) {
	char token_a[128];
	char token_b[128];
	
	char* token = token_a;
	char* last_token = token_b;
	
	short last_token_type = TOKEN_NONE;
	short token_type = TOKEN_NONE;
	
	char last_op = 0;
	
	Value v[12];
	unsigned short temp_data[VAR_SIZE * 12 / 2];
	char first_val = 1;
	int i;
	Value* dest = res;
	char func = 0;
	
	for(i = 0; i < 12; i++)
		v[i].data = (unsigned char *)&temp_data[i * VAR_SIZE / 2];
	
	while(start < end) {
		SWAP(token, last_token);
		
		consume_whitespace(&start, end);
		
		if(start >= end)
			break;
		
		if(is_valid_op(*start)) {
			start = get_token(token, start, end, is_valid_op);
			
			if(last_token == TOKEN_NONE || last_op != 0) {
				cprintf(ERROR, "Unexpected op '%s' -> %s", token, line);
				return 0;
			}
			
			token_type = TOKEN_OP;
			last_op = token[0];
		}
		else if(isdigit(*start)) {
			start = get_token(token, start, end, is_valid_digit);
			short type = TYPE_INT;
			
			if(start < end) {
				if(*start == 'L') {
					type = TYPE_LONG;
					++start;
				}
				else if(*start == 'c') {
					type = TYPE_CHAR;
					++start;
				}
			}
			
			long_as_value(dest, atol(token), type);
			token_type = TOKEN_NUM;
		}
		else if(isalpha(*start)) {
			start = get_token(token, start, end, is_valid_func_char);
			token_type = TOKEN_ID;
		}
		else if(*start == '(') {
			short par = 0;
			
			char* begin = start + 1;
			char* stop;
			short arg = 0;
			
			do {
				consume_whitespace(&start, end);
				
				if(*start == '(') {
					par++;
				}
				else if((*start == ',' && par == 1) || (*start == ')' && par == 1)) {
					stop = start;
					
					if(begin >= stop || !execute_statement(line, begin, stop, &v[arg++]))
						return 0;
					
					begin = start + 1;
				}
				
				if(*start == ')')
					par--;
				
				start++;
			} while(par != 0 && start < end);
			
			if(par != 0) {
				cprintf(ERROR, "Unmatched '(' -> %s", line);
				return 0;
			}
			
			consume_whitespace(&start, end);
			
			if(last_token_type == TOKEN_ID) {
				
				if(!execute_function(line, last_token, dest, v, arg)) {
					return 0;
				}
			}
			else {
				copy_val(&v[arg - 1], dest);
			}
			
			token_type = TOKEN_PAR;
		}
		else if(*start == '"') {
			++start;
			
			dest->type = TYPE_STRING;
			int pos = 0;
			
			while(start < end && *start != '"') {
				dest->data[pos++] = *start++;
			}
			
			token_type = TOKEN_STRING;
			dest->data[pos] = '\0';
		}
		else if(*start == ')') {
			cprintf(ERROR, "Unopened ')' -> %s", line);
			return 0;
		}
		else {
			cprintf(ERROR, "Unexpected char '%c' -> %s", *start, line);
			return 0;
		}
		
		if(token_type != TOKEN_ID || last_token_type != TOKEN_NONE)
			dest = &v[11];
			
		if(last_op != 0 && token_type != TOKEN_OP) {
			if(last_op == '+') {
				long a = value_as_long(res);
				long b = value_as_long(dest);
				
				long_as_value(res, a + b, TYPE_INT);
			}
			
			last_op = 0;
		}
		
		last_token_type = token_type;
	}
	
	return 1;
}











#if 0
char execute_statement(char* line, char* start, char *end, Value* res) {
	Value v[10];
	
	unsigned short temp_data[VAR_SIZE * 10 / 2];
	int i;
	
	if(start >= end) {
		cprintf(ERROR, "Expected value -> %s", line);
		return 0;
	}
	
	char data[VAR_SIZE + 1];
	short data_pos = 0;
	
	for(i = 0; i < 10; i++) {
		v[i].data = (unsigned char *)&temp_data[i * VAR_SIZE / 2];
	}
	
	while(start < end) {
		data[data_pos] = '\0';
		
		consume_whitespace(&start, end);
		
		if(start >= end)
			break;
		
		if(*start == '(') {
			short par = 0;
			
			char* begin = start + 1;
			char* stop;
			short arg = 0;
			
			do {
				consume_whitespace(&start, end);
				
				if(*start == '(') {
					par++;
				}
				else if((*start == ',' && par == 1) || (*start == ')' && par == 1)) {
					stop = start;
					
					if(begin >= stop || !execute_statement(line, begin, stop, &v[arg++]))
						return 0;
					
					begin = start + 1;
				}
				
				if(*start == ')')
					par--;
				
				start++;
			} while(par != 0 && start < end);
			
			if(par != 0) {
				cprintf(ERROR, "Unmatched '(' -> %s", line);
				return 0;
			}
			
			consume_whitespace(&start, end);
			
			if(!execute_function(line, data, res, v, arg))
				return 0;
			else if(start < end) {
				cprintf(ERROR, "Expected end of statement -> %s", line);
			}
			
			return 1;
			
			++start;
			
			
		}
		else if(*start == ')') {
			cprintf(ERROR, "Unopened ')' -> %s", line);
			return 0;
		}
		else if(*start == '"') {
			++start;
			
			res->type = TYPE_STRING;
			
			while(start != end && *start != '"') {
				res->data[data_pos++] = *start++;
			}
			
			res->data[data_pos] = '\0';
			
			return 1;
		}
		else if(isdigit(*start) || *start == '-') {
			char long_val = 0;
			char float_val = 0;
			
			while(start < end && (isdigit(*start) || *start == '-' || *start == 'L' || *start == 'f')) {
				if(*start == 'L')
					long_val = 1;
				else if(*start == 'f')
					float_val = 1;
				else
					data[data_pos++] = *start;
				
				++start;
			}

			data[data_pos] = '\0';
			if(long_val) {
				*((long *)res->data) = atol(data);
				res->type = TYPE_LONG;
			}
			else {
				*((short *)res->data) = atoi(data);
				res->type = TYPE_INT;
			}
			
			return start == end;
		}
		else {
			if(*start != ' ') {
				data[data_pos++] = *start;
			}
			
			++start;
		}
	}
	
	cprintf(ERROR, "Unknown value -> %s", line);
	
	return 1;
}
#endif

char execute_string(char* str) {
	Value res;
	
	short data[VAR_SIZE / 2];
	
	res.data = (unsigned char *)data;
	
	return execute_statement(str, str, str + strlen(str), &res);
}




#if 0
void execute_cmd(char* cmd) {
	char args[1024];
	short arg_pos = 0;
	
	char* argv[20] = {args};
	int argc = 1;
	
	while(*cmd) {
		if(*cmd == ' ') {
			do {
				++cmd;
			} while(*cmd == ' ');
			
			args[arg_pos++] = '\0';
			
			if(!*cmd)
				break;
			
			argv[argc++] = args + arg_pos;
		}
		else {
			args[arg_pos++] = *cmd++;
		}
	}
	
	args[arg_pos] = '\0';
	
	int total_cmd = sizeof(cmd_tab) / sizeof(CMDHandler);
	
	int i;
	char function;
	
	for(i = 0; i < total_cmd; i++) {
		if(strcmp(cmd_tab[i].name, argv[0]) == 0) {
			cmd_tab[i].handler(argc, argv);
		}
	}
}
#endif


void show_console() {
	void* save_screen = malloc(LCD_SIZE);
	LCD_save(save_screen);
	
	void* screen = malloc(LCD_SIZE);
	PortSet(screen, 239, 127);
	clrscr();
	
	unsigned char font = FontGetSys();
	FontSetSys(F_4x6);
	
	short line;
	short y = 0;
	unsigned short key;
	char edit_str[70] = "|";
	short edit_pos = 0;
	char is_edit;
	
redraw:
	line = console_line_head;
	clrscr();
	y = 0;
	
	while(line != console_line_tail) {
		DrawStr(0, y, console_line[line].line, console_line[line].cat != ERROR ? A_NORMAL : A_REVERSE);
		
		line = (line + 1) % MAX_LINES;
		y += 8;
	}
	DrawStr(0, LCD_HEIGHT - 12, edit_str, A_NORMAL);
	DrawLine(0, LCD_HEIGHT - 14, LCD_WIDTH - 1, LCD_HEIGHT - 14, A_NORMAL);

	do {
		memset(screen + (LCD_HEIGHT - 12) * LCD_WIDTH / 8, 0, LCD_WIDTH * 8 / 8);
		DrawStr(0, LCD_HEIGHT - 12, edit_str, A_NORMAL);
		LCD_restore(screen);
		
		key = get_console_key(&is_edit);
			
		if(is_edit) {
			edit_str[edit_pos++] = key;
			edit_str[edit_pos] = '|';
			edit_str[edit_pos + 1] = '\0';
		}
		else {
			if(key == KEY_ESC)
				goto done;
			else if(key == KEY_BACKSPACE) {
				if(edit_pos != 0) {
					edit_str[--edit_pos] = '|';
					edit_str[edit_pos + 1] = '\0';
				}
			}
			else if(key == KEY_CLEAR) {
				edit_pos = 0;
				edit_str[edit_pos] = '|';
				edit_str[edit_pos + 1] = '\0';
			}
			else if(key == KEY_ENTER) {
				edit_str[edit_pos] = '\0';
				cprintf(CMD, edit_str);
				//execute_cmd(edit_str);
				execute_string(edit_str);
				goto redraw;
			}
		}
		
	} while(1);
	
done:
	PortRestore();
	FontSetSys(font);
	LCD_restore(save_screen);
	free(save_screen);
	free(screen);
}


void cprint_line(short category, char* data) {
	ConsoleLine* line = &console_line[console_line_tail];
	
	strncpy(line->line, data, LINE_LENGTH - 1);
	line->line[LINE_LENGTH - 1] = '\0';
	line->cat = category;
	
	console_line_tail = (console_line_tail + 1) % MAX_LINES;
	
	if(console_line_tail == console_line_head)
		console_line_head = (console_line_head + 1) % MAX_LINES;
}

void cprintf_function(const char* func, short category, const char* format, ...) {
	char buf_temp[512];
	char buf[512];
	
	int len;
	va_list list;
	
	va_start(list, format);
	vsprintf(buf_temp, format, list);
	
	if(*func)
		sprintf(buf, "%6s [%12s]: %s", console_cat_name[category], func, buf_temp);
	else
		sprintf(buf, "%6s: %s", console_cat_name[category], buf_temp);
	
	
	len = strlen(buf);
	
	//printf("\n\n\n\nLength: %d\n", len);
	//ngetchx();
	
	char *data = buf;
	while(len > 0) {
		cprint_line(category, data);
		data += LINE_LENGTH - 1;
		len -= LINE_LENGTH - 1;
	}
}

void test_console() {
	init_console();
	
	int i;
	
	for(i = 0; i < 50; i++) {
		cprintf_function(__FUNCTION__, PRINT, "%d", i);
	}
	
	cprintf_function(__FUNCTION__, ERROR, "Hello world!");
	cprintf_function(__FUNCTION__, PRINT, "{1, 2, 3}");
	cprintf_function(__FUNCTION__, ERROR, "Fixed point overflow");
	
	
	show_console();
}

