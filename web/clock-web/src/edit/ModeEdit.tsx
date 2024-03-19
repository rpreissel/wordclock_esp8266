import { Button, Form } from "react-bootstrap";
import { Configs, Mode } from "../types";

import { ModeBaseEdit } from "./ModeBaseEdit";
import { ModeIntervalEdit } from "./ModeIntervalEdit";
import { ModePictureEdit } from "./ModePictureEdit";
import { ModeTimerEdit } from "./ModeTimerEdit";
import { ModeWordClockEdit } from "./ModeWordClockEdit";
import { useState } from "react";

type ModeEditProps = {
  mode: Mode;
  modes: Mode[];
  configs: Configs;
  onSave: (mode: Mode) => void;
};

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
    formContent = <>
      <ModeBaseEdit mode={newMode} colorLabel="Color1" {...props} onChange={mode => setState({ mode, changed: true })} />
      <ModePictureEdit mode={newMode} {...props} onChange={mode => setState({ mode, changed: true })} />
    </>
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
