# PWM timer: Season simulation

Timed and season aware Arduino PWM dimming, e.g. to controll aquarium lighting.

## Features

- Moonlight simulation incorporating seasons and lunar cycles

## Requirements

- Arduino board with [Bridge](https://www.arduino.cc/en/Reference/YunBridgeLibrary) capabilities, e.g. YUN, Industrial 101, etc.

## Usage

Adjust relevant variables and upload to your board.

## Settings

The sketch contains several adjustable parameters:

#### `BAUT_BRIDGE`

The baut rate for the bridge of your board.

#### `BOOT_DELAY`

Time in `[ms]` to wait before initializing the bridge in order to give the linux kernel time to boot.

#### `DAYS_PER_MONTH`

How many days to assume per month.

#### `LUMINOSITY_MAX_MOONLIGHT`

Maximum PWM value for moonlight. I.e. your moon will not become brighter than this. Set to 255 for maximum brightness.

#### `MAX_MOONLIGHT_HOURS_PER_DAY`

Maximum hours of moonlight per day.

#### `PIN__MOONLIGHT`

The PWM pin to use for moonlight simulation.

#### `SKEW_START_OF_YEAR_MOONLIGHT_LUMINOSITY`

This is used to adjust the lunar cycle based on location. To find a suitable value for your desired simulation location it is easiest to plot the `getHoursOfMoonlight()` function and adjust until satisfied (`4.2f ==` Rio De Janeiro).

```
f(x) = (MAX_MOONLIGHT_HOURS_PER_DAY / PI) * asin(cos(PI * x / DAYS_PER_MONTH + SKEW_START_OF_YEAR_MOONLIGHT_LUMINOSITY)) + (MAX_MOONLIGHT_HOURS_PER_DAY / 2)
```

#### `SKEW_MONTHLY_MOONLIGHT_LUMINOSITY`

This is used to adjust the lunar cycle based on location. To find a suitable value for your desired simulation location it is easiest to plot the `getHourOfMoonrise()` function and adjust until satisfied (`4.3f ==` Rio De Janeiro).

```
f(x) = (HOURS_PER_DAY / PI) * asin(cos(PI * x / DAYS_PER_MONTH + SKEW_MONTHLY_MOONLIGHT_LUMINOSITY)) + (HOURS_PER_DAY / 2)
```

## [Changelog](CHANGELOG.md)

## [License](LICENSE)
