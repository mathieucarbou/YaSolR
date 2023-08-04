The Shelly Solar Diverter/Router

Hello,

I wanted to share with you a small project that I carried out in the recent days as part of the "Shelly IoT Innovation Challenge" and which will surely pick your interest: a solar diverter/router that is quite innovative because it is very simple and based on... Shelly's of course!

I always wanted to find a **simple** way to control the **Loncont LSA-H3P50YB** voltage regulator, which is interesting because in addition to being sturdy, it includes a Zero-Cross Detection module and is controlled very easily by voltage variation or PWM.
In particular, it can be controlled by an ESP32 but using an external module and an additional 12V power supply, which is not very practical.

A few days ago, a friend pointed out to me that Shelly has a 0-10V dimmer, the voltage range required to control this regulator, but for it to work, the dimmer must be of the "sourcing current" type and not "sinking current".

Except... Shelly has just released a new dimmer very recently: the **Shelly Dimmer 0/1-10V PM Gen3**, which is precisely of a "sourcing current" type!

What a joy!

So I decided to write a Shelly script which allows you to automatically control this type of voltage regulator via the Shelly Dimmer 0/1-10V PM Gen3, depending on the injection or consumption read from the Shelly EM Pro (purchased from Quintium), which also reads the output power of the regulator with its 2nd current clamp.

I have not yet been able to test in the long term, but the small tests carried out for the moment show that it works, within the possible limit of the precision that can be obtained using the Shelly scripts: reading the measurements at each second and then a call to adjust the dimmer immediately afterwards.

It is therefore a promising solution for the moment, and which remains easy to improve.

Also, dimmers tend to get hot, so I suspect it needs to be installed not within an enclosure.

Features and benefits of this diverter/router:

- Shelly and LSA Loncont components are robust, compliant with standards, and used in industry
- Very easy to set up (Shelly script)
- Automatic divert management via a PID controller which supports several proportional and derivative modes
- Supports a bypass contactor (to force a heating), which will automatically cut off the dimmers if turned on
- Supports a DS1820 temperature probe via the Shelly Add-on to get the water tank temperature (or anything else)
- Support for up to N dimmers, with possible sharing of the excess power between dimmers
- And of course, everything you can have with remote control of Shelly's in the Shelly App, Home Assistant / Jeedom, etc.

It's then up to you to write your Shelly automations to program forced operation, start or stop automatic routing remotely, etc.
Full of possibilities with Shelly's!

The script can be downloaded and modified as you wish (it is under the MIT license), and it can be found on the blog of the YaSolR site, the routing software I've been working for several months now:

https://yasolr.carbou.me/blog/2024-07-01_shelly_solar_diverter

Happy hacking!
