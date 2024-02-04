def forEach(array,function) {
    for (i=0;i<array.size();i++)
        function(array[i]) 
}

files = []
system("grep -r Parse.c")["out"].split("\n").forEach((e) => {
    echo(e.type())
    files.push(e.split(":")[0])
})
files.echo()
//def t(n, x) { return [n,x] }
//m = "m".t(":")[0]
//m.echo()