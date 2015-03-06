# Arduino Lighting Controller

This code is the firmware for lighting installation designed and built by (SODIdesigns)[http://sodidesigns.com/]. It uses 7 (wifire16)[http://propaneandelectrons.com/projects/wifire16] boards chained together to control up to 112 lights. The first board in the chain has a (WiFly)[wi://www.sparkfun.com/products/10822] installed.

By default this firmware displays an animation. It also accepts data from the WiFly and will display that directly. If it hasn't received any valid messages from the WiFly for a while it will go back to showing the default animation.
