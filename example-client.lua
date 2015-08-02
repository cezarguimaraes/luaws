require 'socket'
require 'luaws'

ws = luaws.connect('ws://127.0.0.1:8080')

if(not ws) then
	error('Could not connect to websocket server')
end

print("Type away:\n\tTyping nothing closes the client.")

while(ws:getReadyState() ~= luaws.CLOSED) do
	local input = io.read()

	if(input == '') then 		
		break 
	end

	ws:send(input)	

	ws:poll(2000)

	ws:poll(2000)

	local message = ws:receive()
	if(message) then
		print('Received: ' .. message)
	end
end

ws:close()