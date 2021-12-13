<img width="50" src="./web/public/logo.svg">

# htmpd
 > Warning: htmpd is still under development

lightweight [MPD](https://www.musicpd.org/) (Music Player Daemon) web client written in C, utilizing Websockets, svelte and tailwind

Inspired by [ympd](https://www.ympd.org/) but with modern ui

![2021-12-08-222617_1340x653_scrot](https://user-images.githubusercontent.com/41364823/145271790-bb274af1-6984-4ef2-8827-1075167336c1.png)

## Requirements

To compile and run `htmpd` you need:

* [mpd](http://www.musicpd.org/) itself ofcourse :)
* [ffmpeg](https://www.ffmpeg.org/) or [libav](https://libav.org/)
* [libmpdclient](https://www.musicpd.org/libs/libmpdclient/)

## Quick Start

Compile

```bash
make
```
...then start

```bash
./htmpd
```

Then navigate to [localhost:8000](http://localhost:8000/)

## Contribution
Feel free to open a pull request!

## Features/Bugs
Please create an issue :)

## Copyright
[MIT License](https://opensource.org/licenses/MIT)

## TODOs
- [x] Control mpd functions (play, pause, etc.)
- [ ] Set mpd options (repeat, random, etc.)
- [ ] Browse filesystem
- [ ] Queue management
- [ ] Playlist management
- [ ] Advanced search
- [x] Embedded albumart
- [ ] MPD http stream
- [ ] And more...
