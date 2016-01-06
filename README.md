

## Р§зг

```
f = function() {
    i = "hello world";
    return function() {
        return i;
    };
}();

print(f(), "\n");

```

`run$ ulcer test.ul`

`output$ hello world`