luaws
=====

Really simple and straightforward WebSocket client library for Lua.

Requirements
============

- Lua 5.1 or LuaJIT 2.0
- LuaSocket (not needed to build the library, only to use it)
- [easywsclient](https://github.com/dhbaird/easywsclient)

Usage
=====

```lua
require("socket")
require("luaws")

-- Connect to a WebSocket server:
local ws = luaws.connect('ws://127.0.0.1:80')

-- With origin:
local ws = luaws.connect('ws://127.0.0.1:80', 'http://origin.example')

-- Check if the connection was successful:
if(not ws) then
	error("Could not connect to websocket server")
end

-- Send TEXT messages:
ws:send("Hello World!")

-- Send a BINARY message:
ws:sendBinary(vstruct.pack('3*u1', {255, 0, 0}))

-- ws:send() and ws:sendBinary() do *not* block and do *not* send the message 
-- The messages are buffered and sent later by calling ws:poll()

-- PS: If you are interest in a pure Lua library for packing and unpacking binary data,
-- take a look at https://github.com/ToxicFrog/vstruct

-- Receive a message (buffered by ws:poll()) and pass it to a handler function
-- ws:receive() also do *not* block since the message (if present) were previously buffered by ws:poll()
local message = ws:receive()
if(message) then
	-- Parse message
end

-- Perform the actual network IO:
-- Won't block at all if timeout is 0 (the default)
ws:poll(timeout = 0) -- timeout in milliseconds

-- Get the current WebSocket state:
local state = ws:getReadyState()
-- state is one of the following:
--  luaws.OPEN, luaws.CONNECTING, luaws.CLOSING or luaws.CLOSED

-- Close the WebSocket (sends a CLOSE message and then close() the actual socket)
ws:close()

```

Event Handling Example
======================

This is how I've used luaws while developing a game client.
```lua
local ws = luaws.connect(url, origin)
if(not ws) then
	error('Could not connect to websocket server')
end	

-- Loop while the websocket is connected
while(ws:getReadyState() ~= luaws.CLOSED) do
	-- Poll for network IO events
	-- Previous messages buffered by ws:send() will be sent here, when the socket is ready.
	ws:poll(500)
	-- Parse any incoming messages
	local message = ws:receive()
	if(message) then
		messageHandler(message)
	end

	-- Check for requests from the main game thread and pass them to another handler
	if(in_channel:peek()) then
		requestHandler(in_channel:pop())
	end
end

-- Close websocket
ws:close()
```

Testing
=======

```
# Launch a test echo server
node example-server.js

# Launch the example client
lua example-client.lua

# Type away:
easywsclient: connecting: host=127.0.0.1 port=8080 path=/
Connected to: ws://127.0.0.1:8080
Type away:
        Typing nothing closes the client.
>>> hello world
Received: hello world
```





