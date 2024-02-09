def forEach(array,function) {
for (i=0;i<array.size();i++)
function(array[i]) 
}


while (1) {
    //system("ls -l")["out"]
    "mmm\nmmm".split("\n").forEach((e)=>{echo(1)})
}

//def filter(array, function) {
//    retList = []
//    for (i=0;i<array.size();i++){
//        store = function(array[i])
//        if (store != 0) 
//            retList.push(store)
//    }
//    return retList
// }

//desiredLineNumber = 9
//months = []
//counts = []
//map = {}
//path = 0
//for (k = 0 ; k < 10 ; k++) {
//    path = stdin("Input a path > ")
//    system("ls -l " + path)["out"].split("\n").filter((e)=>{
//        arr = e.split(" ")
//        if (e.split(" ").size() == desiredLineNumber) { return arr }
//    }).forEach((e)=> {
//        echo(e)
//        month = e[5]
//        for (i=0; i < months.size() ;i++) {
//            if (months[i] == month) {
//                counts[i] = counts[i] + 1
//                return
//            }
//        }
//        months.push(month)
//        counts.push(0)
//    })
//
//    for (i = 0 ; i < months.size() ; i++){
//        map[counts[i]] = months[i]
//    }
//
//    map.clear()
//}