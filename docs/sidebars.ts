import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

const sidebars: SidebarsConfig = {
  langSidebar: [
    'intro',
    'getting-started',
    {
      type: 'category',
      label: 'Language Reference',
      items: [
        'language/types',
        'language/variables',
        'language/functions',
        'language/operators',
        'language/control-flow',
        'language/structs',
        'language/arrays',
        'language/inline-asm',
        'language/comments',
        'language/modules',
      ],
    },
  ],
};

export default sidebars;
