import os
import re
import argparse
from jinja2 import FileSystemLoader, Environment

TEMPLATE_HEADER_FILE = "service_h.template"
TEMPLATE_SOURCE_FILE = "server_cc.template"


def name_convert(name: str) -> str:
    out_name = re.sub(r'(_[a-z])', lambda x: x.group(1)[1].upper(), name)
    out_name = out_name[0].upper() + out_name[1:]
    return out_name


class Service(object):
    def __init__(self, idx: int, name: str, input_name: str, input_type: str, output_name: str, output_type: str):
        self.idx = idx
        self.name = name
        self.input_name = input_name
        self.input_type = input_type
        self.output_name = output_name
        self.output_type = output_type


class ProtocolParser(object):
    def __init__(self, pc_path: str, output_dir: str):
        if not os.path.isfile(pc_path):
            print(pc_path, " does not exist")
            exit(1)
        if not os.path.exists(output_dir):
            print(output_dir, " does not exist and will be make by this code")
            os.mkdir(output_dir, 0o777)
        self.pc_path = pc_path
        self.service_list = []
        self.namespace = "lpc"
        self.namespace_text = ""
        self.fb_files = []
        self.generated_fb_headers = []
        pc_name = self.pc_path.split('.')[-2]
        self.header_file_name = pc_name + "_service.h"
        self.header_file_path = os.path.join(output_dir, self.header_file_name)
        self.source_file_path = os.path.join(output_dir, pc_name + "_server.cc")
        self.server_name = name_convert(pc_name) + "Server"
        self.client_name = name_convert(pc_name) + "Client"
        curr_dir = os.path.dirname(__file__)
        self.template_env = Environment(
            loader=FileSystemLoader(searchpath=os.path.join(curr_dir, "../lpc/templates")))
        self.parsed = False

    def parse_syntax(self):
        with open(self.pc_path, "r") as f:
            lines = f.readlines()
        trimed = []
        for line in lines:
            if "import" in line:
                self.fb_files.append(line)
                continue
            if "namespace" in line:
                self.namespace_text = line.strip()
                continue
            trimed.append(line)
        service_text = ""
        for line in trimed:
            service_text += (line.strip().replace(' ', ''))
        services = re.split('service', service_text)
        assert (len(services) > 1)
        return services[1:]

    def parse_import(self, text: str):
        text_split = text.split(' ')[-1]
        return text_split.split('.')[-2] + "_generated.h"

    def parse_service(self, text_list: list):
        for i, text in enumerate(text_list):
            match_obj = re.match(
                r'(.*){(.*)\((.*)\)returns(.*)\((.*)\)}', text, re.M | re.I)
            if match_obj:
                self.service_list.append(Service(i, match_obj.group(1), match_obj.group(
                    2), match_obj.group(3), match_obj.group(4), match_obj.group(5)))
            else:
                print("do not match: ", text)

    def parse(self):
        # Step1: seperate protocol by syntaxes: "import"/"namespace"/"service"
        service_text = self.parse_syntax()
        # Step2: generate generated_fb_files by "import"
        for fb in self.fb_files:
            self.generated_fb_headers.append(self.parse_import(fb))
        # Step3: generate namespace by "namespace", NOTE namespace should be an unique string
        if len(self.namespace_text) > 0:
            self.namespace = self.namespace_text.split(' ')[-1]
        print("namespace: ", self.namespace)
        # Step4: generate service by "service"
        self.parse_service(service_text)
        print("service num: ", len(self.service_list))
        self.parsed = True

    def generate_header_file(self):
        if not self.parsed:
            print("Cannot call generate_header_file before protocol parsed")
            return
        template = self.template_env.get_template(TEMPLATE_HEADER_FILE)
        output_text = template.render(
            protocol_file_name=self.pc_path,
            generated_fb_headers=self.generated_fb_headers,
            namespace=self.namespace,
            client_name=self.client_name,
            service_list=self.service_list,
            server_name=self.server_name,
        )
        with open(self.header_file_path, "w") as f:
            f.write(output_text)

    def generate_source_file(self):
        if not self.parsed:
            print("Cannot call generate_source_file before protocol parsed")
            return
        template = self.template_env.get_template(TEMPLATE_SOURCE_FILE)
        output_text = template.render(
            generated_fb_headers=self.generated_fb_headers,
            generated_header=self.header_file_name,
            namespace=self.namespace,
            service_list=self.service_list,
            server_name=self.server_name,
        )
        with open(self.source_file_path, "w") as f:
            f.write(output_text)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--pc_path", required=True, help="protocol file path")
    parser.add_argument("--out_dir", required=False, default="./",
                        help="where the generated files will locate")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    pc_parser = ProtocolParser(args.pc_path, args.out_dir)
    pc_parser.parse()
    pc_parser.generate_header_file()
    pc_parser.generate_source_file()
