import { ColorMap, Configs, FixedTime, LiveViewData } from "./types";
import { useEffect, useState } from "react"

import { Form } from "react-bootstrap";
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
  fixedTime: FixedTime;
  configs: Configs;
  onChange: (fixedTime: FixedTime) => void
};
const LiveView = ({ configs: { colors, leds }, fixedTime, onChange }: LiveViewProps) => {

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
  const hours = fixedTime.enabled ? fixedTime.hours : data.time.hours;
  const minutes = fixedTime.enabled ? fixedTime.minutes : data.time.minutes;
  return (
    <>
      <div>
        <Form >
          <Form.Group controlId="formTimer" className="mb-3">
            <div className="d-flex flex-wrap">
              <div className="me-3">
                <Form.Control type="time"
                  size="sm"
                  value={hours.toString().padStart(2, '0') + ":" + minutes.toString().padStart(2, '0')}
                  readOnly={!fixedTime.enabled}
                  onChange={e => {
                    const [newHour, newMinute] = e.currentTarget.value.split(":");
                    onChange({enabled:true, hours:+newHour, minutes:+newMinute})

                  }} />
              </div>
              <div className="p-1">
                <Form.Check // prettier-ignore
                  type="switch"
                  label="Fix Time"
                  checked={fixedTime.enabled}
                  onChange={() => {
                    if(fixedTime.enabled) {
                      onChange({enabled:false, hours:fixedTime.hours, minutes:fixedTime.minutes})
                    } else {
                      onChange({enabled:true, hours:hours, minutes:minutes})
                    }
                  }}
                />
              </div>
            </div>
          </Form.Group>
          <Form.Group className="livegrid">
            {range(0, 11).map((row) =>
              <LiveRow key={row} textString={leds[row.toString(16)]} colorString={data.colors[row.toString(16)]} colors={colors} />
            )}
            <LiveRow key="M" textString={leds.M} colorString={"   " + data.colors.M} colors={colors} />
          </Form.Group>
        </Form>
      </div>

    </>
  )
}

export default LiveView;