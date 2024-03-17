import { BaseMode, Configs, IntervalMode, Mode, TimerMode, WordClockMode } from "./types";
import { Button, Form } from "react-bootstrap";

import { ColorChooser } from "./ColorChooser";
import { useState } from "react";

type ModeEditProps = {
  mode: Mode;
  modes: Mode[];
  configs: Configs;
  onSave: (mode: Mode) => void;
};

type ModeBaseEditProps<M extends BaseMode> = {
  mode: M;
  configs: Configs;
  onChange: (mode: M) => void;
};

function ModeBaseEdit<M extends BaseMode>({ mode, configs, onChange }: ModeBaseEditProps<M>) {
  return <>
    <Form.Group className="mb-3" controlId="formBrightness">
      <Form.Label>Brightness</Form.Label>
      <Form.Range value={mode.brightness} min="1" max="255" onChange={e => {
        onChange({ ...mode, brightness: e.currentTarget.value });
      }} />
    </Form.Group>
    <Form.Group className="mb-3" controlId="formColor">
      <Form.Label>Color</Form.Label>
      <ColorChooser value={mode.color} colors={configs.colors} onChange={c =>
        onChange({ ...mode, color: c })
      } />
    </Form.Group>
  </>;
}
type ModeWordClockEditProps = {
  mode: WordClockMode;
  configs: Configs;
  onChange: (mode: WordClockMode) => void;
};
function ModeWordClockEdit({ mode, configs, onChange }: ModeWordClockEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formDefs">
      <Form.Label>Times</Form.Label>
      {Object.keys(configs.times).map((timedIndex, i) => {
        const go = configs.times[timedIndex];
        return <Form.Select key={i} value={mode.times[timedIndex]} className="mb-1" onChange={e => {
          const times = { ...mode.times }
          times[timedIndex] = e.currentTarget.value;
          console.log(times);
          onChange({ ...mode, times: times });
        }}>
          {go.map((t, i) => <option key={i} value={t}>{t}</option>)}
        </Form.Select>
      })}
    </Form.Group>
  </>;
}

type ModeIntervalEditProps = {
  mode: IntervalMode;
  modes: Mode[];
  configs: Configs;
  onChange: (mode: IntervalMode) => void;
};
function ModeIntervalEdit({ mode, modes, onChange }: ModeIntervalEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formInterval">
      <div className="d-flex">
        <div className="d-inline-block w-25 p-1">
          <Form.Label>Seconds</Form.Label>
        </div>
        <div className="d-inline-block flex-fill p-1">
          <Form.Label>Mode</Form.Label>
        </div>
      </div>
      {mode.intervals.map((interval, i) => {
        return (<div key={i} className="d-flex">
          <div className="d-inline-block w-25 p-1">
            <Form.Control type="number" value={interval.seconds} onChange={e => {
              const newinterval = { ...interval, seconds: +e.currentTarget.value }
              const newIntervals = [...mode.intervals];
              newIntervals[i] = newinterval;
              onChange({ ...mode, intervals: newIntervals });
            }} />
          </div>
          <div className="d-inline-block flex-fill p-1">
            <Form.Select value={interval.mode} onChange={e => {
              const newinterval = { ...interval, mode: +e.currentTarget.value }
              const newIntervals = [...mode.intervals];
              newIntervals[i] = newinterval;
              onChange({ ...mode, intervals: newIntervals });
            }}>
              {
                modes
                  .filter((m) => m.type !== "OFF")
                  .filter((m) => m.type !== "EMPTY")
                  .filter(m => m.index != mode.index)
                  .map(m => m as BaseMode)
                  .map((m, mi) => {
                    return <option key={mi} value={m.index}>{m.name}</option>;
                  })

              }
              <option key={-1} value={-1}>Off</option>;
            </Form.Select>
          </div>
          <div className="d-inline-block w-25 p-1">
            <Button className="action float-end" type="button" variant="danger" disabled={mode.intervals.length < 3} onClick={() => {
              const newIntervals = [...mode.intervals]
              newIntervals.splice(i, 1);
              onChange({ ...mode, intervals: newIntervals });
            }}>
              Delete
            </Button>
          </div>
        </div>);
      })}
      <div className="d-flex justify-content-end">
        <div className="d-inline-block w-25 p-1">
          <Button type="button" variant="info" className="action float-end" onClick={() => {
            const newIntervals = [...mode.intervals]
            newIntervals.push({ seconds: 10, mode: -1 });
            onChange({ ...mode, intervals: newIntervals });
          }}>
            Add
          </Button>
        </div>
      </div>
    </Form.Group>
  </>;
}

type ModeTimerEditProps = {
  mode: TimerMode;
  modes: Mode[];
  configs: Configs;
  onChange: (mode: TimerMode) => void;
};
function ModeTimerEdit({ mode, modes, onChange }: ModeTimerEditProps) {
  return <>
    <Form.Group className="mb-3" controlId="formTimer">
      <div className="d-flex flex-wrap">
        <div className="d-inline-block w-50 p-1">
          <Form.Label>Start</Form.Label>
        </div>
        <div className="d-inline-block w-50 p-1">
          <Form.Label>End</Form.Label>
        </div>
      </div>
      {mode.timers.map((timer, i) => {
        return (<div key={i} className="d-flex flex-wrap ">
          <div className="d-inline-block w-50 p-1">
            <Form.Control type="time" value={timer.startHour.toString().padStart(2, '0') + ":" + timer.startMinute.toString().padStart(2, '0')} onChange={e => {
              const [newHour, newMinute] = e.currentTarget.value.split(":");
              const newTimer = { ...timer, startHour: +newHour, startMinute: +newMinute }
              const newTimers = [...mode.timers];
              newTimers[i] = newTimer;
              onChange({ ...mode, timers: newTimers });
            }} />
          </div>
          <div className="d-inline-block w-50 p-1">
            <Form.Control type="time" value={timer.endHour.toString().padStart(2, '0') + ":" + timer.endMinute.toString().padStart(2, '0')} onChange={e => {
              const [newHour, newMinute] = e.currentTarget.value.split(":");
              const newTimer = { ...timer, endHour: +newHour, endMinute: +newMinute }
              const newTimers = [...mode.timers];
              newTimers[i] = newTimer;
              onChange({ ...mode, timers: newTimers });
            }} />
          </div>
          <div className="d-inline-block flex-fill p-1">
            <Form.Select value={timer.mode} onChange={e => {
              const newTimer = { ...timer, mode: +e.currentTarget.value }
              const newTimers = [...mode.timers];
              newTimers[i] = newTimer;
              onChange({ ...mode, timers: newTimers });
            }}>
              {
                modes
                  .filter((m) => m.type !== "OFF")
                  .filter((m) => m.type !== "EMPTY")
                  .filter(m => m.index != mode.index)
                  .map(m => m as BaseMode)
                  .map((m, mi) => {
                    return <option key={mi} value={m.index}>{m.name}</option>;
                  })

              }
              <option key={-1} value={-1}>Off</option>;
            </Form.Select>
          </div>
          <div className="d-inline-block w-25 p-1">
            <Button className="action float-end" type="button" variant="danger" disabled={mode.timers.length < 2} onClick={() => {
              const newTimers = [...mode.timers];
              newTimers.splice(i, 1);
              onChange({ ...mode, timers: newTimers });
            }}>
              Delete
            </Button>
          </div>
        </div>);
      })}
      <div className="d-flex justify-content-end">
        <div className="d-inline-block w-25 p-1">
          <Button type="button" variant="info" className="action float-end" onClick={() => {
            const newTimers = [...mode.timers];
            newTimers.push({ mode: -1, startHour: 0, startMinute: 0, endHour: 1, endMinute: 0 });
            onChange({ ...mode, timers: newTimers });
          }}>
            Add
          </Button>
        </div>
      </div>
    </Form.Group>
  </>;
}

function ModeEdit({ mode, onSave, ...props }: ModeEditProps) {
  const [{ mode: newMode, changed }, setState] = useState({ mode: mode, changed: false });
  if (newMode.type == "EMPTY") {
    return <></>;
  }
  let formContent = <></>;
  if (newMode.type == "DIGICLOCK") {
    formContent = <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />;
  }
  if (newMode.type == "WORDCLOCK") {
    formContent = <>
      <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
      <ModeWordClockEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
    </>;
  }
  if (newMode.type == "PICTURE") {
    formContent = <ModeBaseEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />;
  }
  if (newMode.type == "INTERVAL") {
    formContent = <>
      <ModeIntervalEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
    </>
  }
  if (newMode.type == "TIMER") {
    formContent = <ModeTimerEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />;
  }
  return <Form onSubmit={e => {
    e.preventDefault();
    onSave(newMode);
    setState({ mode: newMode, changed: false });
  }} onReset={e => {
    e.preventDefault();
    setState({ mode, changed: false });
  }}>  {formContent}
    <div className="d-flex justify-content-start" >
      <div className="d-inline-block p-1">

        <Button variant="primary" type="submit" className="action me-2" disabled={!changed}>
          Submit
        </Button>
        <Button variant="secondary" type="reset" className="action" disabled={!changed}>
          Reset
        </Button>
      </div>
    </div>
  </Form>

}

export default ModeEdit;
