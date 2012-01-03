WorkerScript.onMessage = function(message) {
    if (message.action == "pop") {
        WorkerScript.sendMessage({action:"popDone", position: message.position})
    }
}
