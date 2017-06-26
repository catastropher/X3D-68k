// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <string.h>
#include <stdlib.h>

#include "X_Console.h"

void x_consolevar_init(X_ConsoleVar* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig)
{
    x_string_init(&var->stringValue, "");
    
    var->name = name;
    var->type = type;
    var->saveToConfig = saveToConfig;
    var->next = NULL;
    
    x_consolevar_set_value(var, initialValue);
}

void x_consolevar_set_value(X_ConsoleVar* var, const char* varValue)
{
    x_string_assign(&var->stringValue, varValue);
    /// @TODO maybe add type checking?
    
    switch(var->type)
    {
        case X_CONSOLEVAR_INT:
            var->intValue = atoi(varValue);
            break;
            
        case X_CONSOLEVAR_FLOAT:
            var->floatValue = atoi(varValue);
            break;
            
        case X_CONSOLEVAR_STRING:
            // string value was already assigned
            break;
            
        case X_CONSOLEVAR_FP16X16:
            var->fp16x16Value = x_fp16x16_from_float(atof(varValue));
            break;
    }
}

void x_console_init(X_Console* console)
{
    console->consoleVarsHead = NULL;
}

X_ConsoleVar* x_console_get_var(X_Console* console, const char* varName)
{
    X_ConsoleVar* var = console->consoleVarsHead;
    while(var)
    {
        if(strcmp(var->name, varName) == 0)
            return var;
        
        var = var->next;
    }
    
    return NULL;
}

_Bool x_console_var_exists(X_Console* console, const char* name)
{
    return x_console_get_var(console, name) != NULL;
}

void x_console_printf(X_Console* console, const char* format, ...)
{
    
}

void x_console_register_var(X_Console* console, X_ConsoleVar* var)
{
    if(x_console_var_exists(console, var->name))
    {
        x_console_printf(console, "Can't register variable %s, already defined\n", var->name);
        return;
    }
    
    var->next = console->consoleVarsHead;
    console->consoleVarsHead = var;
}

void x_console_set_var(X_Console* console, const char* varName, const char* varValue)
{
    X_ConsoleVar* var = x_console_get_var(console, varName);
    if(!var)
    {
        x_console_printf(console, "Variable %s not found\n", varName);
        return;
    }
    
    x_consolevar_set_value(var, varValue);
}


