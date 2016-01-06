

## 例子

```
f = function() {
    str = "hello world";
    return function() {
        return str;
    };
}();

print(f(), "\n");

```

`run$ ulcer test.ul`

`output$ hello world`
