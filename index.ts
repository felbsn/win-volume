const binary = require('./build/Release/winvolumer.node');

/**
 * set default output device volume
 * @param value 0~1 value that represents current volume level
 */
export function setVolume(value: number) {
    binary.setVolume(value);
}

/**
 * get default output device volume
 * @param value 0~1 value that represents current volume level
 */
export function getVolume(): number {
    return binary.getVolume();
}

export function setMute(value: boolean) {
    return binary.setMute(value);
}

export function getMute(): boolean {
    return binary.getMute();
}

/**
 * combined method for pooling purposes
 * @param value 0~1 value that represents current volume level
 */
export function getVolumeAndMute(): { volume: number, mute: boolean } {
    return binary.getVolumeAndMute();
}

export function setPoolingInterval(miliseconds: number) {

    if (interval)
        clearInterval(interval);
    POOLING_INTERVAL_MS = miliseconds;

    if (interval)
        interval = setInterval(checkVolumeAndMute, POOLING_INTERVAL_MS)


}


type ChangeCallback = (status: { volume: number, mute: boolean }) => void

var POOLING_INTERVAL_MS = 1_000;

var volume_events: ChangeCallback[] = [];
var mute_events: ChangeCallback[] = [];

var volume: number;
var mute: boolean;

var interval: NodeJS.Timer | null = null;

function checkVolumeAndMute() {

    let current = getVolumeAndMute();

    if (current.volume != volume) {
        for (const fn of volume_events) {
            fn(current)
        }
    }
    if (current.mute != mute) {
        for (const fn of mute_events) {
            fn(current)
        }
    }
    ({ volume, mute } = current)
}

export function addEventListener(name: "volumechanged" | "mutechanged", fn: ChangeCallback) {
    if (!interval) {
        ({ volume, mute } = getVolumeAndMute());
        interval = setInterval(checkVolumeAndMute, POOLING_INTERVAL_MS)
    }
    switch (name) {
        case "volumechanged":
            volume_events.push(fn);
            break;
        case "mutechanged":
            mute_events.push(fn);
            break;
        default:
            console.error("invalid event name given " + name)
            break;
    }

}
export function removeEventListener(fn: ChangeCallback) {
    let index = volume_events.indexOf(fn)
    if (index >= 0)
        volume_events.splice(index, 1)

    index = mute_events.indexOf(fn)
    if (index >= 0)
        mute_events.splice(index, 1)

    if (mute_events.length == 0 && volume_events.length == 0 && interval) {
        clearInterval(interval);
    }
}

