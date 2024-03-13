import { useEffect, useState } from "react";

interface ModesModel {
  get modes(): Mode[];
  get configs(): Configs;

  current: number;
}
export function useModes(): [ModesModel | undefined] {
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

  class ModesModelImpl implements ModesModel {
    get modes(): Mode[] {
      return (modes as Modes).modes;
    }
    get configs(): Configs {
      return config as Configs;
    }

    get current(): number {
      return (modes as Modes).current;
    }

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
    }
  }

  return [new ModesModelImpl()];
}
