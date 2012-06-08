break
=====

`break` is a simple C program that forces you to take regular breaks.

Installation
------------

Just issue

    make

and copy `break` somewhere into your `PATH`. GTK 2 is required.

Usage
-----

`break` divides your time into work-break units. Each unit is described
by duration of work and a break. For example

    break 600 60

means 10 minutes of work and a minute break.

You can specify several work-break units to have a more complicated
setting, e.g.:

    break 1200 120 600 60 300 30

means 20 minutes of work, 2 minutes break, 10 minutes of work, 1 minute
break, 5 minutes of work, 30 seconds break. Then the cycle repeats.

In general it accepts pairs of numbers. Each pair describes one
work-break unit. The first number is the duration of work in seconds;
the second number is the duration of a break in seconds.
