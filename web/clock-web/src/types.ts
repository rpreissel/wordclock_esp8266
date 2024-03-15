type Color = {
  name: string;
  r: number;
  g: number;
  b: number;
};

type ColorMap = { [k: string]: Color };

type TimesConfigMap = { [k: string]: string[] };
type TimesMap = { [k: string]: string };
type PixelMap = { [k: string]: string };

type Configs = {
  types: string[];
  colors: ColorMap;
  times: TimesConfigMap;
};

type Ledmatrix = {
  0:string;
  1:string;
  2:string;
  3:string;
  4:string;
  5:string;
  6:string;
  7:string;
  8:string;
  9:string;
  a:string;
  M:string;
}

type LiveViewData = {
  text: Ledmatrix;
  colors: Ledmatrix
}


type IndexMode = {
  index: number;
};

type BaseMode = IndexMode & {
  name: string;
  color: string;
  brightness: number;
};

type EmptyMode = IndexMode & {
  type: "EMPTY";
};

type OffMode = IndexMode & {
  type: "OFF";
};

type WordClockMode = BaseMode & {
  type: "WORDCLOCK";
  times: TimesMap;
};

type DigiClockMode = BaseMode & {
  type: "DIGICLOCK";
};

type PictureMode = BaseMode & {
  type: "PICTURE";
  pixels: PixelMap;
};

type Timer = {
  mode: number;
  startHour: number;
  startMinute: number;
  endHour: number;
  endMinute: number;
};

type TimerMode = BaseMode & {
  type: "TIMER";
  mainMode: number;
  timers: Timer[];
};

type Interval = {
  mode: number;
  seconds: number;
};

type IntervalMode = BaseMode & {
  type: "INTERVAL";
  intervals: Interval[];
};

type FixedTime = {
  enabled: boolean;
  hours: number;
  minutes: number;
};

type Mode = EmptyMode | OffMode | WordClockMode | DigiClockMode | PictureMode | TimerMode | IntervalMode;

type Modes = {
    modes: Mode[];
    current: number;
    fixedTime: FixedTime;
}

type PartialModes = Partial<Modes>

