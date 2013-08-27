Animation Plugin for CryEngine SDK
=====================================
Can be used for face animation or manual bone rotation control. (e.g. live performance stuff)

Can be combined with the flite plugin to provide realtime lipsync based on speech synthesis for characters.

Installation / Integration
==========================
Use the installer or extract the files to your CryEngine SDK Folder so that the Code and BinXX/Plugins directories match up.

The plugin manager will automatically load up the plugin when the game/editor is restarted or if you directly load it.

Flownodes
=========
* ```Animation_Plugin:Facial:Animation``` Will trigger morphs (this is the node to use in combination with Flite Plugin)
 * In ```EntityId``` Character to animate 
 * In ```Start``` Start morphing
 * In ```StopLast``` Stop last morph performed by this node
 * In ```Name``` Morph Name
 * In ```Weight``` Morph Weight
 * In ```FadeDuration``` Fade in Seconds
 * In ```Duration``` Duration in Seconds
 * In ```Repeat``` Repeat
 * Out ```Started``` Morph Started

* ```Animation_Plugin:Facial:MorphX``` Will update/perform a number of morphs (one such node per character is possible)
 * In ```EntityId``` Character to animate
 * In ```Name``` 
 * In ```Weight``` Weight to apply to this 
 * Out ```Weight``` Current Weight 

* ```Animation_Plugin:Bone:RotateX``` Will rotate a number of bones
 * In ```EntityId``` Character to animate
 * In ```Active``` Reapply the data each frame
 * In ```Slot``` Character Slot of the Bones
 * In ```Radiant``` Radiant or degrees
 * In ```RelativeToDefaultPose``` New Data relative to default pose
 * In ```Bone``` Name of the Bone
 * In ```Rotation``` New Rotation of the Bone

* ```Animation_Plugin:Bone:RotPosX``` Will rotate and place  a number of bones
 * In ```EntityId``` Character to animate
 * In ```Active``` Reapply the data each frame
 * In ```Slot``` Character Slot of the Bones
 * In ```Radiant``` Radiant or degrees
 * In ```RelativeToDefaultPose``` New Data relative to default pose
 * In ```Bone``` Name of the Bone
 * In ```Rotation``` New Rotation of the Bone
 * In ```Position``` New Position of the Bone

Contributing
============
* See [Plugin SDK Wiki: Contribution Guideline](https://github.com/hendrikp/Plugin_SDK/wiki/Contribution-Guideline)
* [Wishes / Issues](https://github.com/hendrikp/Plugin_Animation/issues)
