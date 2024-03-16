import { useEffect, useState } from "react";
import { Modes, Configs, Mode, ColorMap, TimesConfigMap, FixedTime } from "./types";

type ModesModel = Readonly<Modes> & Readonly<Configs> & {
  set current(index: number);
  changeMode(mode:Mode):void;
}
export function useModel(): [ModesModel | undefined] {
  const [config, setConfig] = useState<Configs | undefined>();
  const [modes, setModes] = useState<Modes | undefined>();

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
    get modes(): Mode[] {
      return _modes.modes;
    },

    get colors(): ColorMap {
      return _configs.colors;
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
          console.log(data);
        })
        .catch((error) => console.log(error));
    },
  }

  return [model];
}
