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

export type Configs = {
  types: string[];
  colors: ColorMap;
  times: TimesConfigMap;
};

export type Ledmatrix = {
  0: string;
  1: string;
  2: string;
  3: string;
  4: string;
  5: string;
  6: string;
  7: string;
  8: string;
  9: string;
  a: string;
  M: string;
};

export type LiveViewData = {
  text: Ledmatrix;
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

export type FixedTime = {
  enabled: boolean;
  hours: number;
  minutes: number;
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
