# Introduction #

In order for KARI to start tracking your motions, you must stand in the Skeleton Calibration Pose.
So stand up straight, feet apart past your shoulders, elbows out straight sideways, and hands upwards slightly bent in.

The log window will show that you have been calibrated and will start tracking you.


example of a rule/execution set in the config file:


---

```
[set]
[require]
bodyAction	= handLeftForward
distanceMin	= 10
distanceMax	= 20
[execute]
keyTap		= a
[execute]
keyHold		= b
```

---


Available detected body actions:

|turnLeft|turnRight|jump|crouch|
|:-------|:--------|:---|:-----|
|leanForward|leanBackward|leanLeft|leanRight|
|handLeftForward|handLeftDown|handLeftUp|handLeftOut|handLeftAcross|heandLeftBackward|
|handRightForward|handRightDown|handRightUp|handRightOut|handRightAcross|handRightBackward|
|footLeftForward|footLeftOut|footLeftBackward|footLeftUp|footLeftBackward|
|footRightForward|footRightOut|footRightBackward|footRightUp|footRightBackward|

# Require commands #

**[[require](require.md)]** options

|distanceMin|Minimum inches from related joint before considered valid|
|:----------|:--------------------------------------------------------|
|distanceMax|Maximum inches from related joint before considered valid|
|angleMinX  |Minimun X angle from related joint before considered valid|
|angleMaxX  |Maximum X angle from related joint before considered valid|
|angleMinY  |Minimun Y angle from related joint before considered valid|
|angleMaxY  |Maximum Y angle from related joint before considered valid|


# Execute commands #

**[[execute](execute.md)]** options

|**keyHold**|will hold that key the entire time the requirements are still being met.|
|:----------|:-----------------------------------------------------------------------|
|**keyTap** |will execute once each time a new instance of the requirements are met. |
|**keyPress**|will execute once each time a new instance of the requirements are met. |
|**keyRelease**|will execute once each time a new instance of the requirements are met. |
|**mouseTap**|values: mouseLeft , mouseRight                                          |
|**mouseHold**|values: mouseLeft , mouseRight                                          |
|**mouseTrack**|values: absolute, relative, push, drag                                  |
|**windowHold**|values: absolute, relative, push, drag                                  |
|**windowShow**|values: maximize, minimize, hide                                        |
|**configLoad**|eg: configLoad = sf4.cfg                                                |

**mouseTrack** and **windowHold** sensitivity is scaled using hands distance between **distanceMin** and **distanceMax**. With near to body being fast moving, and far from body being more precise.

eg:

---

```
[set]
[require]
bodyAction	= handRightForward
distanceMin	= 17
distanceMax	= 30
angleMinX	= 1
angleMaxX	= 180
angleMinY	= 1
angleMaxY	= 180
[execute]
mouseTrack	= drag
```

---


distance values are considered inches.
angle values are considered degrees.



**Extended keys**


|back|tab|return|shift|control|
|:---|:--|:-----|:----|:------|
|menu|pause|capslock|
|escape|space|end   |home |
|arrow\_left|arrow\_up|arrow\_right|arrow\_down|
|print|snapshot|insert|delete|
|win\_left|win\_right|
|numpad\_0|numpad\_1|numpad\_2|numpad\_3|numpad\_4|
|numpad\_5|numpad\_6|numpad\_7|numpad\_8|numpad\_9|
|multiply|add|separator|subtract|decimal|divide |
|fkey\_1|fkey\_2|fkey\_3|fkey\_4|fkey\_5|fkey\_6|
|fkey\_7|fkey\_8|fkey\_9|fkey\_10|fkey\_11|fkey\_12|
|numlock|scroll|
|shift\_left|shift\_right|control\_left|control\_right|menu\_left|menu\_right|