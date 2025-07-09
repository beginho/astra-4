-- Simple admin web UI

local addr = admin_addr or "0.0.0.0"
local port = admin_port or 8090
local root = admin_root or "admin"

local start_time = os.time()

local function api_stats(server, client, request)
    if not request then return nil end
    local data = { uptime = os.time() - start_time }
    server:send(client, {
        code = 200,
        headers = {"Content-Type: application/json", "Connection: close"},
        content = json.encode(data)
    })
end

local function api_control(server, client, request)
    if not request then return nil end
    local cmd = request.query and request.query.cmd
    if cmd == "stop" then
        server:send(client, {
            code = 200,
            headers = {"Content-Type: application/json", "Connection: close"},
            content = json.encode({ result = "stopping" })
        })
        astra.exit()
    else
        server:send(client, {
            code = 200,
            headers = {"Content-Type: application/json", "Connection: close"},
            content = json.encode({ result = "unknown" })
        })
    end
end

http_server({
    addr = addr,
    port = port,
    route = {
        { "/api/stats", api_stats },
        { "/api/control", api_control },
        { "/*", http_static({ path = root }) }
    }
})

