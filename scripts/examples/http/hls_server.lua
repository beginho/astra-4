-- Example HTTP HLS server

local input = udp_input({ addr = "239.0.0.1", port = 1234 })

http_hls({
    upstream = input:stream(),
    path = "./hls",
    segment = 5,
    playlist = 5
})

http_server({
    addr = "0.0.0.0",
    port = 8080,
    route = {
        { "/hls/*", http_static({ path = "./hls" }) }
    }
})

