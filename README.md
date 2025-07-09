# Description

Astra (Advanced Streamer) is a professional software to organize Digital TV Service for
TV operators and broadcasters, internet service providers, hotels, etc.

* Open source version not maintained. Please, check https://cesbo.com/astra/ to get more info.
* Telegram Community: [EN](https://t.me/cesbo_en) [RU](https://t.me/cesbo_ru)

## HLS Output

Use `http_hls` to segment an incoming MPEG-TS stream and expose the files via
`http_static`. An example script is available at
`scripts/examples/http/hls_server.lua`.

## Admin Web UI

Run `scripts/admin.lua` to start a small administration interface. A ready to
use example is `scripts/examples/admin_server.lua`. The web page is served from
the `admin` directory.
