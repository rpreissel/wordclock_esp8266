import { ColorMap, Configs, LiveViewData } from "./types";
import { useEffect, useState } from "react"

import { colorIndexToHex } from "./colors";
import { range } from "./range";

type LiveRowProps = {
  colors: ColorMap;
  textString: string;
  colorString: string;
};

const LiveRow = ({ textString, colorString, colors }: LiveRowProps) => {
  return <>
    {range(0, 11).map((col) => {
      const color = col < colorString.length && colorString[col] != ' ' ? colorIndexToHex(+colorString[col], colors) : undefined;
      return <div key={col} className="livecell" style={{ color: color }}>
        {color && col < textString.length ? textString[col] : ' '}
      </div>;
    })
    }
  </>
}

type LiveViewProps = {
  configs: Configs;
};
const LiveView = ({ configs: {colors, leds} }: LiveViewProps) => {

  const [data, setData] = useState<LiveViewData | undefined>()

  useEffect(() => {
    function loadLiveView() {
      fetch('./api/live')
        .then(result => result.json())
        .then(result => setData(result))
    }
    loadLiveView()
    const interval = setInterval(() => loadLiveView(), 1000)
    return () => {
      clearInterval(interval);
    }
  }, [])

  if (!data) {
    return <></>;
  }
  return (
    <div className="livegrid">
      {range(0, 11).map((row) => 
        <LiveRow key={row} textString={leds[row.toString(16)]} colorString={data.colors[row.toString(16)]} colors={colors} />
      )}
      <LiveRow key="M" textString={leds.M} colorString={"   " + data.colors.M} colors={colors} />
    </div>

  )
}

export default LiveView;