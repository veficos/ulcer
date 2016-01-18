
##函数

`ulcer`的函数声明同javascript一样。

```
function funcname(a, b, c, d) {
}
```

如果一个`function`没有返回值，那么它将默认返回一个`null`值。

同样，`ulcer`的函数也带有词法定界（lexical scoping）的第一类值（first-class values）。所以，在`ulcer`当中可以把一个`function`直接赋值给一个变量。

```
f = function(){};
```

```
function new_counter() {
    i = 0;
    return function() {
        i++;
        return i;
    }
}

c1 = new_counter();
print(c1(), "\n");
print(c1(), "\n");
```

程序运行结果是：

```
1
2
```

##流程控制

`ulcer`支持`if/elif/else`和`switch`两种循环控制方式，语法同javascript/C类似。

```
if (condition) {
} elif (condition) {
} else {
}
```
```
switch (v) {
case 'helloworld':
    {
    }
default:
    {
    }
}
```

`ulcer`支持`for`，`foreach`和`while`三种循环控制，其中`for`和`while`语法同javascript类似。

```
for (i = 0; i < 10; i++) {
}
```

```
while (true) {
}
```

`ulcer`另外提供了`foreach`循环控制用于迭代array和table类型。

```
foreach(k, v : table) {
}
```

##类型

`ulcer`支持以下几种类型，其中，array和table属于引用类型，而其它属于值类型。

```
vnull = null;
vbool = true;
vint = 1;
vlong = 1l;
vfloat = 1.0f;
vdouble = 1.0;
vstring = "HelloWorld";
varray = [1,2,3,4,5];
vtable = {1:vint, "hello":"world"};
vfunction = function(){};
vpointer = file.open("test.txt", "r");
```

##包管理

`ulcer`的包管理同lua类似。

```
// fibonacci.ul

function fibonacci(n) {
    if(n == 1) {
        return 1;
    }
    return fibonacci( n - 1 ) * n;
}
```

```
// main.ul
import "fibonacci"
print(fibonacci(10), "\n");
```

`import`关键字只是简单地将`fibonacci`这个包作为源文件运行，然后将`fibonacci`运行结果保存到`main`的上下文当中。

## 原型链

这个功能我还在思量着要不要加上，对于一门动态语言来说，似乎没什么必要。

