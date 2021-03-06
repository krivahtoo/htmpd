<img width="50" src="./web/public/logo.svg">

# htmpd
 > Warning: htmpd is still under development you might experience bugs or incomplete features

lightweight [MPD](https://www.musicpd.org/) (Music Player Daemon) web client written in C, utilizing Websockets, svelte and tailwind

Inspired by [ympd](https://www.ympd.org/) but with modern ui

![2022-06-29-145217_1325x656_scrot](https://user-images.githubusercontent.com/41364823/177026834-5ec4da40-ed95-4927-bb58-cd17630e272d.png)
![2021-12-17-115732_383x657_scrot](https://user-images.githubusercontent.com/41364823/146580516-60100bbc-72d1-4c16-8f66-d88645a79192.png)


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
[GPL 3.0](https://opensource.org/licenses/GPL-3.0)

## TODOs
- [x] Control mpd functions (play, pause, etc.)
- [x] Set mpd options (repeat, random, etc.)
- [ ] Browse filesystem
- [ ] Queue management
- [ ] Playlist management
- [ ] Advanced search
- [x] Embedded albumart
- [ ] MPD http stream
- [ ] And more...
