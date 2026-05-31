# PianoFromAbove

This is the software that drives some of the Impossible MIDI videos on youtube:

<p align="center">
  <a href="https://www.youtube.com/watch?v=p_c6uQHlhZ0" target="_blank">
    <img src="https://img.youtube.com/vi/p_c6uQHlhZ0/hqdefault.jpg"/>
  </a>
</p>

Synthesia is the main alternate with way more market share and much more active development, but appaerently PFA is more performant, so some prefer it. Yay.

The original inspiration:

<p align="center">
  <a href="https://www.youtube.com/watch?v=mTS16klgqMU" target="_blank">
    <img src="https://img.youtube.com/vi/mTS16klgqMU/hqdefault.jpg"/>
  </a>
</p>

And so I made it happen:


<p align="center">
  <a href="https://www.youtube.com/watch?v=PWQj61p6D5s" target="_blank">
    <img src="https://web.archive.org/web/20250425015407im_/https://camo.githubusercontent.com/2152ea7d1710e9966dc0159cc7610c745d04b0b6cecf213f0482686a1052477e/68747470733a2f2f696d672e796f75747562652e636f6d2f76692f5057516a363170364435732f687164656661756c742e6a7067"/>
  </a>
</p>

## Binaries

https://github.com/brian-pantano/PianoFromAbove/releases

## How to build

This is unfortunately very tricky. Hopefully I will simplify this in the future.

* Clone this repo
* Download and install Visual Studio 2022
* Download and install DirectX SDK (Jun 2010)
* ~Download and extract Google Protocol Buffers 2.5~ (already [included](https://github.com/KarbitsCode/PianoFromAbove/tree/master/ThirdVendor))
  * Build `libprotobuf-lite.vcxproj` (or by opening `protobuf-lite.slnf` file)
* ~Download and extract Boost 1.55~ (not needed anymore)
* Open the main `PianoFromAbove.sln` file ~and edit the VC++ Directories from the project properties so that the Include Directories and Library Directories point to the location of your boost and protocol buffers downloads~ (already configured)
* Cross fingers
* Build! (Release, x64)

Once that's done, there should be a `Release\KPFA-1.1.0-x86_64.exe` that you can run.

There's an optional .nsi script that you can run if you want to build an installer.

The code probably isn't the best, and it probably goes against all sorts of best practices but it is fairly snappy. I'm not very good at writing UI or UX, but I am fairly good at writing datastructures and writing minimal and fast code. Good luck reading it!
