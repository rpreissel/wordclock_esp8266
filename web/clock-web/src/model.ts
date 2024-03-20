import { useEffect, useState } from "react";
import { Modes, Configs, Mode, ColorMap, TimesConfigMap, FixedTime, Ledmatrix } from "./types";

type ModesModel = Readonly<Modes> & Readonly<Configs> & {
  get version():number;
  set current(index: number);
  set fixedTime(fixedTime: FixedTime);
  changeMode(mode:Mode):void;
  changeModes(modes:{ index: number, type: string, name: string }[]):void;
  get currentMode(): Mode;
}
export function useModel(): [ModesModel | undefined] {
  const [config, setConfig] = useState<Configs | undefined>();
  const [modes, setModes] = useState<Modes | undefined>();
  const [version, setVersion] = useState<number>(0);

  useEffect(() => {
    fetch("./api/configs", {
      method: "GET",
    })
      .then((response) => response.json())
      .then((data) => {
        setConfig(data);
        console.log(data);
      })
      .catch((error) => console.log(error));
  }, []);
  useEffect(() => {
    fetch("./api/modes", {
      method: "GET",
    })
      .then((response) => response.json())
      .then((data) => {
        setModes(data);
        setVersion(v=> v+1);
        console.log(data);
      })
      .catch((error) => console.log(error));
  }, []);

  if (!config || !modes) {
    return [undefined];
  }

  const _modes = modes as Modes;
  const _configs = config as Configs;
  const model: ModesModel = {
    get version():number {
      return version;
    },
    get modes(): Mode[] {
      return _modes.modes;
    },

    get colors(): ColorMap {
      return _configs.colors;
    },
    get leds(): Ledmatrix {
      return _configs.leds;
    },

    
    get types() : string[] {
        return _configs.types;
    },

    get times() : TimesConfigMap {
        return _configs.times;
    },
    

    get current(): number {
      return _modes.current;
    },

    
    get fixedTime() : FixedTime {
      return _modes.fixedTime;
    },
    

    set current(index: number) {
      fetch("./api/modes", {
        method: "PATCH",
        body: JSON.stringify({ current: index }),
      })
        .then((response) => response.json())
        .then((data) => {
          setModes(data);
          setVersion(version+1);
          console.log(data);
        })
        .catch((error) => console.log(error));
    },

    set fixedTime(fixedTime: FixedTime) {
      fetch("./api/modes", {
        method: "PATCH",
        body: JSON.stringify({ fixedTime: fixedTime }),
      })
        .then((response) => response.json())
        .then((data) => {
          setModes(data);
          console.log(data);
        })
        .catch((error) => console.log(error));
    },

    changeMode(mode:Mode):void {
      fetch("./api/modes", {
        method: "PATCH",
        body: JSON.stringify({ modes: [mode] }),
      })
        .then((response) => response.json())
        .then((data) => {
          setModes(data);
          setVersion(version+1);
          console.log(data);
        })
        .catch((error) => console.log(error));
    },

    changeModes(modes:{ index: number, type: string, name: string }[]):void {
      fetch("./api/modes", {
        method: "PATCH",
        body: JSON.stringify({ modes }),
      })
        .then((response) => response.json())
        .then((data) => {
          setModes(data);
          setVersion(version+1);
          console.log(data);
        })
        .catch((error) => console.log(error));
    },
    
    get currentMode(): Mode {
      if(this.current<0) {
        return {type:"OFF", index: this.current};
      }
      return this.modes[this.current];
    }
    
  }

  return [model];
}
