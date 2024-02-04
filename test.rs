

def forEach(array,function) {
    for (i=0;i<array.size();i++)
        function(array[i]) 
}

files = []
system("grep -r Parse.c 2> /dev/null")["out"].split("\n").forEach((e) => { 
    echo(files.push(e.split(":")[0]))
})


files.echo()