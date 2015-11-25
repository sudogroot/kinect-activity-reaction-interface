#Mouse configuration breakdown

# Introduction #

Here are some examples and info about setting up your mouse config.


# Details #

**Angles**

Sense your hand won't be tracked outside of the distance or angle boundaries you setup, this also affects how sensitive your hand movement is compared to what you display on the screen. So movement of your hand when fully extended, will be easier to control than movement with your hand closer to your body.


**Relative:**

This means that the hand movement you do, will map in pixels to how big your screen is, starting from wherever your hand started to get tracked.

```
[set]
[require]
bodyAction	= handLeftForward
distanceMin	= 18
angleMinX	= 60
angleMaxX	= 120
angleMinY	= 60
angleMaxY	= 120
[execute]
windowHold	= relative
```

**Absolute**
This distance movement tracks the same as relative, except it will directly place your cursor or window where you are pointing in relationship to your hands position in your usage boundaries, directly compared to the screen.

```
[set]
[require]
bodyAction	= handRightForward
distanceMin	= 18
angleMinX	= 60
angleMaxX	= 120
angleMinY	= 60
angleMaxY	= 120
[execute]
mouseTrack	= absolute
```

**Drag**
This is so you can do slow drags of your target across the screen. Best used for video game precision styled aiming, etc.

```
[set]
[require]
bodyAction	= handRightForward
distanceMin	= 18
angleMinX	= 60
angleMaxX	= 120
angleMinY	= 60
angleMaxY	= 120
[execute]
mouseTrack	= drag
```

**Push**
This will slowly push the cursor in the direction your hand is compared to the horizontal/vertical position of your shoulder. So your hand straight forward won't move at all, but put your hand to the right of center, and the mouse will drift to the right.

```
[set]
[require]
bodyAction	= handRightForward
distanceMin	= 18
angleMinX	= 60
angleMaxX	= 120
angleMinY	= 60
angleMaxY	= 120
[execute]
mouseTrack	= push
```