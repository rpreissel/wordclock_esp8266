import { Accordion, AccordionContext, Container } from "react-bootstrap";
import { PropsWithChildren, useContext } from "react";

import LiveView from "./LiveView.tsx";
import ModeEdit from "./edit/ModeEdit.tsx";
import ModeThumbnail from "./ModeThumbnail.tsx";
import ModesManage from "./ModesManage.tsx";
import { modeName } from "./types.ts";
import { useModel } from "./model.ts";

type AccordionBodyExtProps = {
  eventKey:string;
};
const AccordionBodyExt =({eventKey, children}:PropsWithChildren<AccordionBodyExtProps>) => {
  const { activeEventKey } = useContext(AccordionContext);
  return <Accordion.Body>
    {activeEventKey && (activeEventKey as string[]).includes(eventKey) ? children : <></>}
  </Accordion.Body>
};
function App() {
  const [model] = useModel();
  console.log("render")
  if (!model) {
    return <div>Loading</div>;
  }

  return (
    <Container fluid >
      <Accordion defaultActiveKey={["Modes"]} alwaysOpen>
        <Accordion.Item eventKey="Modes">
          <Accordion.Header>Modes</Accordion.Header>
          <Accordion.Body>

            <div className="d-flex justify-content-center">
              <div style={{ width: "fit-content", margin: "auto" }}>

                <ModeThumbnail key={-1} mode={{ type: "OFF", index: -1 }} colors={model.colors} active={model.current == -1}
                  onClick={index => model.current = index} />
                {model.modes.map((mode) => (

                  <ModeThumbnail key={mode.index} mode={mode} colors={model.colors} active={model.current == mode.index}
                    onClick={index => model.current = index} />

                ))}
              </div>
            </div>
          </Accordion.Body>
        </Accordion.Item>
        {model.current >= 0 && (<>
          <Accordion.Item eventKey="Configure">
            <Accordion.Header>Configure '{modeName(model.currentMode)} / {model.currentMode.type}'</Accordion.Header>
            <AccordionBodyExt eventKey="Configure">
                <div className="d-flex justify-content-center">
                  <ModeEdit key={model.currentMode.type + model.current} mode={model.currentMode} configs={model} modes={model.modes} onSave={(mode) => model.changeMode(mode)} />
                </div>
            </AccordionBodyExt>
          </Accordion.Item>
        </>)}
        <Accordion.Item eventKey="View">
          <Accordion.Header>View Clock</Accordion.Header>
          <AccordionBodyExt eventKey="View">
              <div className="d-flex justify-content-center">
                <LiveView configs={model} fixedTime={model.fixedTime} onChange={fixedTime => model.fixedTime=fixedTime} />
              </div>
          </AccordionBodyExt>
        </Accordion.Item>
        <Accordion.Item eventKey="Organize">
          <Accordion.Header>Organize Modes</Accordion.Header>
          <AccordionBodyExt eventKey="Organize">
              <div className="d-flex justify-content-center">
                <ModesManage key={model.version} modes={model.modes.map(m => ({ index: m.index, type: m.type, name: modeName(m) }))}
                  configs={model} onSave={modes => model.changeModes(modes)} />
              </div>
          </AccordionBodyExt>
        </Accordion.Item>
      </Accordion>
    </Container>
  );
}

export default App;
