# Programming_Language
Simple programing language created with C, lex and yacc

Running on Windows:
```
flex lang.l
bison -dy lang.y
gcc lex.yy.c y.tab.c src/scope.c src/statement.c -o lang.exe
lang.exe <input
```

Input file specified in ```lang.exe <input``` contains language code

Sample code:
```
print("Testing arithmetics");

var a: int = 100;
var b: int = 25;
var c: int = 4;
var d: int = 20;

a = 100 * b;
b = b + d;
c = a / 25;
d = 15 + 30;

print(a);
print(b);
print(c);
print(d);

print("Testing function");

function foo(param1: int, param2: string, param3: bool): bool {
	print(param1);
	print(param2);
	print(param3);
	
	var count: int = 0;
	var maxCount: int = 5;
	
	loop count < maxCount {
		var returnCount: int = 3;
		
		count = count + 1;
		
		print(count);
		
		if count is returnCount {
			print("returning false");
			return false;
		}			
	}
	
	print("returning true");
	
	return true;
}

var result: bool = true;

result = foo(21, "my age", false);

print(result);

print("Testing if statement");

var cond1: bool = true;
var cond2: bool = false;

if cond1 or cond2 {
	print("Yay, inside if statement");
	
	foo(20, "my previous age", true);
	
	if not cond2 {
		print("condition inversion seems to be working");
	}
}

print("Testing loop");

var count: int = 0;
var maxCount: int = 10;
var continueConditionValue: int = 5;
var breakConditionValue: int = 7;

loop count < maxCount {
	count = count + 1;
	
	if count is continueConditionValue {
		if cond1 {
			if not cond2 {
				print("to be continued");
				continue;
			}
		}
	}
	
	print(count);
	
	if count is breakConditionValue {
		print("break dance");
		break;
	}
}

print("Testing default return value");

function vadaba(num: int): int {
	var something: int = 5;
	var zero: int = 0;
	var result: int = 0;
	
	result = something / zero;
	
	return something;
}

var returnedValue: int = 0;
var defaultReturnValue: int = 3;

returnedValue = *defaultReturnValue vadaba(10);

print(returnedValue);
```
