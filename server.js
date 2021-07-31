// Node.js WebSocket server script

const Http = require("http");
const WebSocketServer = require("websocket").server;

let board = [ ['-', '-', '-'], ['-', '-', '-'], ['-', '-', '-'] ];
let numMoves = 0;

const server = Http.createServer();
server.listen(8080, 
    function() 
    {
        console.log((new Date()) + " Server is listening on port 8080");
    }
);

const wsServer = new WebSocketServer({httpServer: server});

function isValidMove(i, j)
{
    return (board[i-1][j-1] == '-') ? true : false;
}

function checkWin(letter) 
{
    return (
    (board[0][0] == letter && board[1][0] == letter && board[2][0] == letter) ||
    (board[0][1] == letter && board[1][1] == letter && board[2][1] == letter) ||
    (board[0][2] == letter && board[1][2] == letter && board[2][2] == letter) ||
    (board[0][0] == letter && board[0][1] == letter && board[0][2] == letter) ||
    (board[1][0] == letter && board[1][1] == letter && board[1][2] == letter) ||
    (board[2][0] == letter && board[2][1] == letter && board[2][2] == letter) ||
    (board[0][0] == letter && board[1][1] == letter && board[2][2] == letter) ||
    (board[0][2] == letter && board[1][1] == letter && board[2][0] == letter) ) ?
    true : false;
}

function printBoard()
{
    console.log("board[0][0]: " + board[0][0] + "\tboard[0][1]: " + board[0][1] + "\tboard[0][2]: " + board[0][2]);
    console.log("board[1][0]: " + board[1][0] + "\tboard[1][1]: " + board[1][1] + "\tboard[1][2]: " + board[1][2]);
    console.log("board[2][0]: " + board[2][0] + "\tboard[2][1]: " + board[2][1] + "\tboard[2][2]: " + board[2][2]);
}

wsServer.on("request", 
    function(request) 
    {
        const connection = request.accept(null, request.origin);
        console.log((new Date()) + " Connection accepted.");

        connection.on("message", 
            function(message) 
            {
                if (message.type === "utf8") 
                {
                    console.log("Received Message: " + message.utf8Data);

                    // Do Work
                    let letter = 'X';
                    let ijStr = message.utf8Data;
                    let i = parseInt(ijStr.charAt(0));
                    let j = parseInt(ijStr.charAt(1));

                    if (isValidMove(i, j))
                    {
                        board[i-1][j-1] = letter;
                        numMoves++;
                        printBoard();
                    }
                    else
                    {
                        connection.sendUTF("failure");
                        return;
                    }

                    let won = checkWin(letter);
                    if (numMoves == 9)
                        connection.sendUTF("tie");
                    else if (won)
                        connection.sendUTF("won");
                    else
                        connection.sendUTF(board.toString());
                }
                else if (message.type === "binary") 
                {
                    console.log("Received Binary Message of " + message.binaryData.length + " bytes");
                    connection.sendBytes(message.binaryData);
                }
            }
        );

        connection.on("close", 
            function(reasonCode, description) 
            {
                console.log((new Date()) + " Peer " + connection.remoteAddress + " disconnected.");
                console.log("Reason Code: " + reasonCode + "\nDescription: " + description);
            }
        );
    }
);