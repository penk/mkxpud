#!/bin/bash
dbus-send --system --print-reply --dest=org.moblin.connman / org.moblin.connman.Manager.GetProperties
