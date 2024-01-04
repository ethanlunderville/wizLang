def test(param, other_param) {
    t = param + other_param
    if (t > 5) {
        return t
    }
    return 1
}

x = test(6,2)
println(x);