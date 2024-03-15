import { useEffect, useState } from "react"

import { colorIndexToHex } from "./colors";

const range = (start: number, end: number) => Array.from({ length: (end - start) }, (_, k) => k + start);
type LiveRowProps = {
  colors: ColorMap;
  text: string;
  colorString: string;
};

const LiveRow = ({ text, colorString, colors }: LiveRowProps) => {
  return <>
    {range(0, 11).map((index) => {
      const color = index < colorString.length && colorString[index] != ' ' ? colorIndexToHex(+colorString[index], colors) : undefined;
      return <div className="livecell" style={{ color: color }}>
        {index < text.length ? text[index] : ' '}
      </div>;
    })
    }
  </>
}

type LiveViewProps = {
  colors: ColorMap;
};
const LiveView = ({ colors }: LiveViewProps) => {

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
      <LiveRow text={data.text['0']} colorString={data.colors['0']} colors={colors} />
      <LiveRow text={data.text['1']} colorString={data.colors['1']} colors={colors} />
      <LiveRow text={data.text['2']} colorString={data.colors['2']} colors={colors} />
      <LiveRow text={data.text['3']} colorString={data.colors['3']} colors={colors} />
      <LiveRow text={data.text['4']} colorString={data.colors['4']} colors={colors} />
      <LiveRow text={data.text['5']} colorString={data.colors['5']} colors={colors} />
      <LiveRow text={data.text['6']} colorString={data.colors['6']} colors={colors} />
      <LiveRow text={data.text['7']} colorString={data.colors['7']} colors={colors} />
      <LiveRow text={data.text['8']} colorString={data.colors['8']} colors={colors} />
      <LiveRow text={data.text['9']} colorString={data.colors['9']} colors={colors} />
      <LiveRow text={data.text.a} colorString={data.colors.a} colors={colors} />
      <LiveRow text={"   " + data.text.M} colorString={"   " + data.colors.M} colors={colors} />
    </div>

  )
}

export default LiveView;