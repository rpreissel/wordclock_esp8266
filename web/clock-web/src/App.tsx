import { Accordion, AccordionContext, Container } from "react-bootstrap";
import { PropsWithChildren, useContext } from "react";

import LiveView from "./LiveView.tsx";
import ModeEdit from "./edit/ModeEdit.tsx";
import ModeThumbnail from "./ModeThumbnail.tsx";
import ModesManage from "./ModesManage.tsx";
import { modeName } from "./types.ts";
import { useModel } from "./model.ts";

type AccordionPartProps = {
  eventKey: string;
  header: string
};
const AccordionPart = ({ eventKey, header, children }: PropsWithChildren<AccordionPartProps>) => {
  const { activeEventKey } = useContext(AccordionContext);
  return <Accordion.Item eventKey={eventKey}>
    <Accordion.Header>{header}</Accordion.Header>
    <Accordion.Body>
      {activeEventKey && (activeEventKey as string[]).includes(eventKey) ? children : <></>}
    </Accordion.Body>
  </Accordion.Item>
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
        <AccordionPart eventKey="Modes" header="Modes">
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
        </AccordionPart>
        {model.current >= 0 && (<>
          <AccordionPart eventKey="Configure" header={`Configure '${modeName(model.currentMode)} / ${model.currentMode.type}'`}>
            <div className="d-flex justify-content-center">
              <ModeEdit key={model.currentMode.type + model.current} mode={model.currentMode} configs={model} modes={model.modes} onSave={(mode) => model.changeMode(mode)} />
            </div>
          </AccordionPart>
        </>)}
        <AccordionPart eventKey="View" header="View Clock">
            <div className="d-flex justify-content-center">
              <LiveView modes={model.modes} configs={model} fixedTime={model.fixedTime} onChange={fixedTime => model.fixedTime = fixedTime} />
            </div>
        </AccordionPart>
        <AccordionPart eventKey="Organize" header="Organize Modes">
            <div className="d-flex justify-content-center">
              <ModesManage key={model.version} modes={model.modes.map(m => ({ index: m.index, type: m.type, name: modeName(m) }))}
                configs={model} onSave={modes => model.changeModes(modes)} />
            </div>
        </AccordionPart>
      </Accordion>
    </Container>
  );
}

export default App;
