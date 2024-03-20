export type Color = {
  name: string;
  r: number;
  g: number;
  b: number;
};

export type ColorMap = { [k: string]: Color };

export type TimesConfigMap = { [k: string]: string[] };

export type TimesMap = { [k: string]: string };

export type PixelMap = { [k: string]: string };

export type Ledmatrix = { [k: string]: string };

export type Time = {
  hours: number;
  minutes: number;
};

export type FixedTime = Time & {
  enabled: boolean;
};

export type Configs = {
  types: string[];
  colors: ColorMap;
  times: TimesConfigMap;
  leds: Ledmatrix;
};


export type LiveViewData = {
  time: Time;
  colors: Ledmatrix;
};

export type IndexMode = {
  index: number;
};

export type BaseMode = IndexMode & {
  name: string;
  color: string;
  brightness: number;
};

export type EmptyMode = IndexMode & {
  type: "EMPTY";
};

export type OffMode = IndexMode & {
  type: "OFF";
};

export type WordClockMode = BaseMode & {
  type: "WORDCLOCK";
  times: TimesMap;
};

export type DigiClockMode = BaseMode & {
  type: "DIGICLOCK";
};

export type PictureMode = BaseMode & {
  type: "PICTURE";
  pixels: PixelMap;
  color1: string;
  color2: string;
};

export type Timer = {
  mode: number;
  startHour: number;
  startMinute: number;
  endHour: number;
  endMinute: number;
};

export type TimerMode = BaseMode & {
  type: "TIMER";
  mainMode: number;
  timers: Timer[];
};

export type Interval = {
  mode: number;
  seconds: number;
};

export type IntervalMode = BaseMode & {
  type: "INTERVAL";
  intervals: Interval[];
};


export type Mode =
  | EmptyMode
  | OffMode
  | WordClockMode
  | DigiClockMode
  | PictureMode
  | TimerMode
  | IntervalMode;

export type Modes = {
  modes: Mode[];
  current: number;
  fixedTime: FixedTime;
};

export function modeName(mode:Mode) {
  if(mode.type === "OFF") {
    return "OFF";
  }
  if(mode.type === "EMPTY") {
    return "EMPTY";
  }
  return mode.name;
}
