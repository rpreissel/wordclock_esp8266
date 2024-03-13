type Color = {
  name: string;
  r: number;
  g: number;
  b: number;
};

type ColorMap = { [k: string]: Color };

type TimeMap = { [k: string]: string[] };

type Configs = {
  types: string[];
  colors: ColorMap;
  times: TimeMap;
};

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
};

type DigiClockMode = BaseMode & {
  type: "DIGICLOCK";
};

type PictureMode = BaseMode & {
  type: "PICTURE";
};

type TimerMode = BaseMode & {
  type: "TIMER";
};

type IntervalMode = BaseMode & {
  type: "INTERVAL";
};

type Mode = EmptyMode | OffMode | WordClockMode | DigiClockMode | PictureMode | TimerMode | IntervalMode;

type Modes = {
    modes: Mode[];
    current: number;
}

type PartialModes = Partial<Modes>

