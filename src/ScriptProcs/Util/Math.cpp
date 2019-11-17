#include "Math.h"
#include "../../vm.h"
#include "../../util.h"
#include <math.h>



void abs(ScriptValue &s, ScriptValue *args)
{
	if (args->type == SCRIPT_LIST && args->listVal->numVals > 0) {
		args[0].listVal->vals[0].AddRef();
		CoerceMath(args[0].listVal->vals[0], s);
		if (s.type == SCRIPT_INT) {
			if (s.intVal < 0) s.intVal = -s.intVal;
		}
		//else if (s.type == SCRIPT_DOUBLE) {
		else if (s.doubleVal < 0) s.doubleVal = -s.doubleVal;
		//}
		return;
	}
	CreateIntValue(s, 0);
}

float __cdecl my_logf (float);
void log (ScriptValue &s, ScriptValue *args)
{
	double dval = args->doubleVal;
	double dlog = my_logf(dval);
	CreateDoubleValue(s, dlog);
}

float __cdecl my_log10f (float);
void log10(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_log10f(args->doubleVal));
}

float __cdecl my_cosf (float);
void cos(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_cosf(args->doubleVal));
}

float __cdecl my_sinf (float);
void sin(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_sinf(args->doubleVal));
}


float __cdecl my_acosf (float);
void acos(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_acosf(args->doubleVal));
}

float __cdecl my_asinf (float);
void asin(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_asinf(args->doubleVal));
}

float __cdecl my_tanf (float);
void tan(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_tanf(args->doubleVal));
}

float __cdecl my_atanf (float);
void atan(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_atanf(args->doubleVal));
}

float __cdecl my_sqrtf (float);
void sqrt(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_sqrtf(args->doubleVal));
}

float __cdecl my_expf (float);
void exp(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_expf(args->doubleVal));
}

float __cdecl my_powf (float, float);
void pow(ScriptValue &s, ScriptValue *args) {
	CreateDoubleValue(s, my_powf(args[0].doubleVal, args[1].doubleVal));
}


void rand(ScriptValue &s, ScriptValue *args) {
	if (args[0].intVal > args[1].intVal) {
		__int64 t = args[0].intVal;
		args[0].intVal = args[1].intVal;
		args[1].intVal = t;
	}
	unsigned __int64 range = args[1].intVal - args[0].intVal;
	if (range == (unsigned __int64) 0) {
		s = args[0];
	}
	else {
		unsigned __int64 v;
		while(1) {
			unsigned __int64 n = GetNumber();
			v = n%range;
			if (((unsigned __int64)(0-range)) > v*range) break;
		}
		CreateIntValue(s, v + args[0].intVal);
	}
}

void randf(ScriptValue &s, ScriptValue *args) {
	if (args[1].doubleVal==args[0].doubleVal) {
		s = args[0];
	}
	else {
		CreateDoubleValue(s, args[0].doubleVal + GetNumber()*(args[1].doubleVal-args[0].doubleVal)/(unsigned __int64)-1);
	}
}
